#pragma once

#include "engine/gpu/memory/GPUBuffer.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace liz {

/// Simulates a GPU VRAM memory pool.
/// Simple bookkeeping — no real GPU memory, no concurrency.
class GPUMemoryPool {
public:
    /// Simulated total VRAM in bytes (default 512 MB).
    static constexpr std::size_t DEFAULT_VRAM = 512ULL * 1024ULL * 1024ULL;

    explicit GPUMemoryPool(std::size_t total_memory = DEFAULT_VRAM);

    /// Allocate a buffer of the given size.
    /// Returns a GPUBuffer with a valid id, or {0,0} on OOM.
    GPUBuffer allocate(std::size_t size);

    /// Free a previously allocated buffer by id.
    void free(std::uint64_t id);

    /// Bytes currently in use.
    std::size_t used_memory() const;

    /// Total VRAM capacity.
    std::size_t total_memory() const;

    /// Number of live buffers.
    std::size_t buffer_count() const;

    /// Log a pool summary via LIZ_INFO.
    void log_summary() const;

private:
    std::size_t              total_memory_;
    std::size_t              used_memory_  = 0;
    std::uint64_t            next_id_      = 1;
    std::vector<GPUBuffer>   allocated_;
};

} // namespace liz