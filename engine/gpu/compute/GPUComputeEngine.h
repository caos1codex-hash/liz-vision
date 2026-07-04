#pragma once

#include "engine/gpu/compute/GPUQueue.h"
#include "engine/gpu/memory/GPUMemoryPool.h"

#include <cstddef>
#include <memory>
#include <string>

namespace liz {

class Batch;  // forward declaration

/// GPU compute orchestrator.
///
/// Receives commands via submit(), executes them sequentially
/// via process_next() / process_all().  Logs "[GPU] executing: X".
///
/// Sprint 8 extension: batch-aware processing.
///   - process_batch() submits commands for an entire Batch at once.
///   - FIFO mode is fully preserved.
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

    // -- FIFO command mode (Sprint 7 — preserved) ------------------------------
    /// Enqueue a command for later processing.
    void submit(std::shared_ptr<GPUCommand> cmd);

    /// Pop and execute one command from the queue.
    /// Returns true if a command was executed, false if empty.
    bool process_next();

    /// Execute all commands currently in the queue.
    void process_all();

    // -- Batch mode (Sprint 8 — new) ------------------------------------------
    /// Submit commands for an entire Batch, then process them all.
    /// Logs "[GPU] Batch #X processing N tensors...".
    void process_batch(const Batch& batch,
                       const std::string& model_name,
                       const std::string& operation);

    /// Process all commands in the queue (alias for process_all,
    /// but with batch-aware logging context).
    void execute_tensor_batch();

    // -- Accessors -------------------------------------------------------------
    GPUQueue&       queue();
    GPUMemoryPool&  memory_pool();

    std::size_t commands_processed() const;
    std::size_t batches_processed() const;
    std::size_t tensors_in_batches() const;
    bool        is_initialized()   const;

private:
    std::unique_ptr<GPUMemoryPool> pool_;
    GPUQueue                     queue_;
    bool                         initialized_         = false;
    std::size_t                  commands_processed_  = 0;
    std::size_t                  batches_processed_  = 0;
    std::size_t                  tensors_in_batches_  = 0;
};

} // namespace liz