#include "engine/gpu/GPUContext.h"
#include "engine/core/Logger.h"

#include <sstream>

namespace liz {

// ── CPUBackend implementation (header-only in this .cpp) ─────────────────────
namespace {

/// CPU fallback backend — all operations happen on the CPU.
/// Simulates the GPU API surface without any GPU hardware.
class CPUBackend final : public liz::GPUBackend {
public:
    std::string_view name() const override { return "CPUBackend"; }

    bool initialize() override {
        initialized_ = true;
        LIZ_INFO("CPUBackend: initialized (CPU fallback mode)");
        return true;
    }

    void shutdown() override {
        // Release all tracked allocations.
        for (auto& [ptr, size] : allocations_) {
            delete[] static_cast<std::uint8_t*>(ptr);
        }
        allocations_.clear();
        initialized_ = false;
        LIZ_INFO("CPUBackend: shut down, all memory released");
    }

    bool is_initialized() const override { return initialized_; }

    GPUMemoryHandle allocate_memory(std::size_t bytes) override {
        if (bytes == 0) return nullptr;
        auto* ptr = new std::uint8_t[bytes];
        allocations_.push_back({ptr, bytes});

        std::ostringstream oss;
        oss << "CPUBackend: allocated " << bytes << " bytes at " << ptr;
        LIZ_TRACE(oss.str());

        return static_cast<GPUMemoryHandle>(ptr);
    }

    void release_memory(GPUMemoryHandle handle) override {
        if (!handle) return;
        auto* ptr = static_cast<std::uint8_t*>(handle);

        for (auto it = allocations_.begin(); it != allocations_.end(); ++it) {
            if (it->ptr == ptr) {
                delete[] ptr;
                allocations_.erase(it);
                LIZ_TRACE("CPUBackend: released memory");
                return;
            }
        }
        LIZ_WARN("CPUBackend: attempted to release unknown handle");
    }

    GPUMemoryHandle upload_frame(const liz::VideoFrame& frame) override {
        auto size = frame.data_size_bytes();
        auto handle = allocate_memory(size);
        if (handle) {
            auto* dst = static_cast<std::uint8_t*>(handle);
            const auto& src = frame.data();
            std::copy(src.begin(), src.end(), dst);
        }

        std::ostringstream oss;
        oss << "CPUBackend: uploaded frame #" << frame.frame_id()
            << " [" << frame.width() << "x" << frame.height() << "] "
            << size << " bytes";
        LIZ_DEBUG(oss.str());

        return handle;
    }

    liz::VideoFrame download_frame(GPUMemoryHandle handle,
                                    std::uint32_t width,
                                    std::uint32_t height) override {
        std::size_t size = static_cast<std::size_t>(width) * height * 3;
        std::vector<std::uint8_t> data(size, 0);

        if (handle) {
            auto* src = static_cast<const std::uint8_t*>(handle);
            std::copy(src, src + size, data.begin());
        }

        return liz::VideoFrame(0, width, height, 0.0, std::move(data));
    }

    GPUMemoryHandle execute_kernel(std::string_view kernel_name,
                                    GPUMemoryHandle input,
                                    std::size_t output_size) override {
        // Simulate: allocate output, copy input (pass-through).
        auto output = allocate_memory(output_size);
        if (output && input) {
            auto* dst = static_cast<std::uint8_t*>(output);
            auto* src = static_cast<const std::uint8_t*>(input);
            auto copy_size = std::min(output_size,
                                      static_cast<std::size_t>(1024 * 1024 * 50)); // cap at 50MB log
            std::copy(src, src + copy_size, dst);
        }

        std::ostringstream oss;
        oss << "CPUBackend: executed kernel '" << kernel_name
            << "' (simulated CPU pass-through, "
            << output_size << " bytes)";
        LIZ_INFO(oss.str());

        return output;
    }

    std::string device_info() const override {
        return "CPU Fallback — No GPU detected";
    }

    std::size_t device_memory_total() const override { return 0; }
    std::size_t device_memory_free() const override { return 0; }

private:
    bool initialized_ = false;
    struct Alloc { void* ptr; std::size_t size; };
    std::vector<Alloc> allocations_;
};

} // anonymous namespace

// ── GPUContext implementation ─────────────────────────────────────────────────
GPUContext::~GPUContext() {
    if (initialized_) {
        shutdown();
    }
}

bool GPUContext::initialize() {
    LIZ_INFO("GPUContext: probing devices...");

    device_probe_.probe();

    if (device_probe_.has_gpu()) {
        // Future: select CUDA/ROCm/Vulkan backend.
        LIZ_INFO("GPUContext: GPU detected — would select GPU backend (future sprint)");
    }

    // For now: always use CPUBackend.
    backend_ = std::make_unique<CPUBackend>();

    if (!backend_->initialize()) {
        LIZ_ERROR("GPUContext: backend initialization failed");
        return false;
    }

    initialized_ = true;
    return true;
}

void GPUContext::shutdown() {
    if (backend_) {
        backend_->shutdown();
        backend_.reset();
    }
    initialized_ = false;
    LIZ_INFO("GPUContext: shut down");
}

bool GPUContext::is_initialized() const {
    return initialized_;
}

std::string GPUContext::backend_name() const {
    return backend_ ? std::string(backend_->name()) : "none";
}

std::string GPUContext::device_info() const {
    return backend_ ? backend_->device_info() : "no backend";
}

std::string GPUContext::backend_type() const {
    return backend_ ? "CPU" : "none";
}

void GPUContext::log_routing_decision() const {
    std::ostringstream oss;
    oss << "GPUContext routing: "
        << "devices=[" << device_probe_.summary() << "] "
        << "selected='" << backend_name() << "' "
        << "info='" << device_info() << "'";
    LIZ_INFO(oss.str());
}

// ── Unified API (delegates to backend) ────────────────────────────────────────
GPUMemoryHandle GPUContext::allocate_memory(std::size_t bytes) {
    if (!backend_) return nullptr;
    return backend_->allocate_memory(bytes);
}

void GPUContext::release_memory(GPUMemoryHandle handle) {
    if (backend_) backend_->release_memory(handle);
}

GPUMemoryHandle GPUContext::upload_frame(const VideoFrame& frame) {
    if (!backend_) return nullptr;
    return backend_->upload_frame(frame);
}

VideoFrame GPUContext::download_frame(GPUMemoryHandle handle,
                                       std::uint32_t width,
                                       std::uint32_t height) {
    static VideoFrame empty;
    if (!backend_) return empty;
    return backend_->download_frame(handle, width, height);
}

GPUMemoryHandle GPUContext::execute_kernel(std::string_view kernel_name,
                                            GPUMemoryHandle input,
                                            std::size_t output_size) {
    if (!backend_) return nullptr;
    return backend_->execute_kernel(kernel_name, input, output_size);
}

} // namespace liz