#pragma once

#include "engine/gpu/GPUBackend.h"
#include "engine/gpu/GPUDevice.h"

#include <memory>
#include <string>

namespace liz {

/// High-level GPU context that manages backend selection and routing.
///
/// Probes the system for available devices, selects the best backend
/// (CPU fallback in this sprint), and exposes a unified API.
///
/// Usage:
///   GPUContext ctx;
///   ctx.initialize();          // auto-selects best backend
///   auto h = ctx.upload(f);    // unified call
///   auto out = ctx.execute("upscale", h, size);
///   ctx.shutdown();
class GPUContext {
public:
    GPUContext() = default;
    ~GPUContext();

    // Non-copyable.
    GPUContext(const GPUContext&) = delete;
    GPUContext& operator=(const GPUContext&) = delete;

    // ── Lifecycle ─────────────────────────────────────────────────────────────
    /// Probe devices and initialize the best available backend.
    /// Returns true if any backend was initialized successfully.
    bool initialize();

    /// Shut down the active backend.
    void shutdown();

    /// True after a successful initialize().
    bool is_initialized() const;

    // ── Device info ───────────────────────────────────────────────────────────
    /// Name of the active backend.
    std::string backend_name() const;

    /// Human-readable device info from the active backend.
    std::string device_info() const;

    /// Which backend type is active.
    std::string backend_type() const;   // "CPU", "CUDA", etc.

    /// Log a summary of detected devices and active selection.
    void log_routing_decision() const;

    // ── Unified GPU API (delegates to active backend) ─────────────────────────
    GPUMemoryHandle allocate_memory(std::size_t bytes);
    void             release_memory(GPUMemoryHandle handle);
    GPUMemoryHandle upload_frame(const VideoFrame& frame);
    VideoFrame       download_frame(GPUMemoryHandle handle,
                                      std::uint32_t width,
                                      std::uint32_t height);
    GPUMemoryHandle execute_kernel(std::string_view kernel_name,
                                    GPUMemoryHandle input,
                                    std::size_t output_size);

private:
    GPUDevice device_probe_;
    std::unique_ptr<GPUBackend> backend_;
    bool initialized_ = false;
};

} // namespace liz