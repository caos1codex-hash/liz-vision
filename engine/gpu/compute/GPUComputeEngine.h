#pragma once

#include "engine/gpu/compute/GPUQueue.h"
#include "engine/gpu/memory/GPUMemoryPool.h"

#include <cstddef>
#include <memory>

namespace liz {

/// GPU compute orchestrator.
///
/// Receives commands via submit(), executes them sequentially
/// via process_next() / process_all().  Logs "[GPU] executing: X".
///
/// No internal concurrency — simple and stable.
class GPUComputeEngine {
public:
    GPUComputeEngine();
    ~GPUComputeEngine() = default;

    // Non-copyable.
    GPUComputeEngine(const GPUComputeEngine&) = delete;
    GPUComputeEngine& operator=(const GPUComputeEngine&) = delete;

    /// Initialize the internal memory pool (512 MB by default).
    bool initialize(std::size_t vram_mb = 512);

    /// Shut down the engine.
    void shutdown();

    /// Enqueue a command for later processing.
    void submit(std::shared_ptr<GPUCommand> cmd);

    /// Pop and execute one command from the queue.
    /// Returns true if a command was executed, false if empty.
    bool process_next();

    /// Execute all commands currently in the queue.
    void process_all();

    /// Accessors.
    GPUQueue&       queue();
    GPUMemoryPool&  memory_pool();

    std::size_t commands_processed() const;
    bool        is_initialized()   const;

private:
    std::unique_ptr<GPUMemoryPool> pool_;
    GPUQueue                     queue_;
    bool                         initialized_        = false;
    std::size_t                  commands_processed_ = 0;
};

} // namespace liz