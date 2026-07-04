#include "engine/gpu/memory/GPUMemoryPool.h"
#include "engine/core/Logger.h"

#include <algorithm>
#include <sstream>

namespace liz {

// -- Constructor --------------------------------------------------------------
GPUMemoryPool::GPUMemoryPool(std::size_t total_memory)
    : total_memory_(total_memory) {
    std::ostringstream oss;
    oss << "GPUMemoryPool: initialized with "
        << (total_memory_ / (1024 * 1024)) << " MB simulated VRAM";
    LIZ_INFO(oss.str());
}

// -- Allocation ---------------------------------------------------------------
GPUBuffer GPUMemoryPool::allocate(std::size_t size) {
    if (size == 0) {
        LIZ_WARN("GPUMemoryPool: allocate(0) — returning empty buffer");
        return {0, 0};
    }

    if (used_memory_ + size > total_memory_) {
        std::ostringstream oss;
        oss << "GPUMemoryPool: OOM — requested " << size
            << " bytes, available=" << (total_memory_ - used_memory_);
        LIZ_ERROR(oss.str());
        return {0, 0};
    }

    GPUBuffer buf{next_id_++, size};
    allocated_.push_back(buf);
    used_memory_ += size;

    std::ostringstream oss;
    oss << "GPUMemoryPool: allocated buffer #" << buf.id
        << " (" << size << " bytes) — used="
        << used_memory_ << "/" << total_memory_;
    LIZ_DEBUG(oss.str());

    return buf;
}

// -- Free ---------------------------------------------------------------------
void GPUMemoryPool::free(std::uint64_t id) {
    auto it = std::find_if(allocated_.begin(), allocated_.end(),
        [id](const GPUBuffer& b) { return b.id == id; });

    if (it == allocated_.end()) {
        std::ostringstream oss;
        oss << "GPUMemoryPool: free(" << id << ") — buffer not found";
        LIZ_WARN(oss.str());
        return;
    }

    used_memory_ -= it->size;
    allocated_.erase(it);

    std::ostringstream oss;
    oss << "GPUMemoryPool: freed buffer #" << id
        << " — used=" << used_memory_ << "/" << total_memory_;
    LIZ_DEBUG(oss.str());
}

// -- Query --------------------------------------------------------------------
std::size_t GPUMemoryPool::used_memory()  const { return used_memory_; }
std::size_t GPUMemoryPool::total_memory() const { return total_memory_; }
std::size_t GPUMemoryPool::buffer_count() const { return allocated_.size(); }

void GPUMemoryPool::log_summary() const {
    std::ostringstream oss;
    oss << "GPUMemoryPool: total="
        << (total_memory_ / (1024 * 1024)) << "MB"
        << " used=" << (used_memory_ / 1024) << "KB"
        << " buffers=" << allocated_.size();
    LIZ_INFO(oss.str());
}

} // namespace liz