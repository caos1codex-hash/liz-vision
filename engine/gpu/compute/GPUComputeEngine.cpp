#include "engine/gpu/compute/GPUComputeEngine.h"
#include "engine/gpu/compute/GPUCommand.h"
#include "engine/gpu/batch/Batch.h"
#include "engine/core/Logger.h"

#include <iostream>
#include <sstream>

namespace liz {

// -- Constructor --------------------------------------------------------------

GPUComputeEngine::GPUComputeEngine() = default;

// -- Lifecycle ----------------------------------------------------------------

bool GPUComputeEngine::initialize(std::size_t vram_mb) {
    if (initialized_) {
        LIZ_WARN("GPUComputeEngine: already initialized");
        return true;
    }

    pool_ = std::make_unique<GPUMemoryPool>(vram_mb * 1024ULL * 1024ULL);
    initialized_ = true;

    std::ostringstream oss;
    oss << "GPUComputeEngine: initialized with "
        << vram_mb << " MB VRAM";
    LIZ_INFO(oss.str());

    return true;
}

void GPUComputeEngine::shutdown() {
    if (!initialized_) return;

    LIZ_INFO("GPUComputeEngine: shutting down...");
    pool_->log_summary();
    pool_.reset();
    initialized_ = false;
    LIZ_INFO("GPUComputeEngine: shut down");
}

// -- FIFO command mode (Sprint 7 — preserved) --------------------------------

void GPUComputeEngine::submit(std::shared_ptr<GPUCommand> cmd) {
    queue_.push(std::move(cmd));
}

bool GPUComputeEngine::process_next() {
    auto cmd = queue_.pop();
    if (!cmd) return false;

    std::cout << "[GPU] executing: " << cmd->name() << std::endl;
    cmd->execute();
    ++commands_processed_;
    return true;
}

void GPUComputeEngine::process_all() {
    while (process_next()) {
        // keep going until queue is empty
    }
}

// -- Batch mode (Sprint 8 — new) ---------------------------------------------

void GPUComputeEngine::process_batch(const Batch& batch,
                                      const std::string& model_name,
                                      const std::string& operation) {
    auto n = batch.size();
    ++batches_processed_;
    tensors_in_batches_ += n;

    std::cout << "[GPU] Batch #" << batch.batch_id
              << " processing " << n << " tensors ("
              << batch.total_bytes() << " bytes)" << std::endl;

    // Submit an upload command per tensor.
    for (std::size_t i = 0; i < n; ++i) {
        std::ostringstream name;
        name << "batch_" << batch.batch_id << "_upload_" << i;
        submit(std::make_shared<UploadFrameCommand>(
            name.str(), batch.get_tensors()[i].byte_size()));
        ++commands_processed_;
    }

    // Submit one inference command for the whole batch.
    {
        std::ostringstream name;
        name << "batch_" << batch.batch_id << "_inference";
        submit(std::make_shared<RunInferenceCommand>(
            name.str(), model_name, operation));
        ++commands_processed_;
    }

    // Submit one copy/download per tensor.
    for (std::size_t i = 0; i < n; ++i) {
        std::ostringstream name;
        name << "batch_" << batch.batch_id << "_copy_" << i;
        submit(std::make_shared<CopyBufferCommand>(
            name.str(), static_cast<std::uint64_t>(i),
            batch.get_tensors()[i].byte_size()));
        ++commands_processed_;
    }

    // Execute all submitted commands for this batch.
    process_all();

    std::cout << "[GPU] Batch #" << batch.batch_id
              << " completed (" << n << " tensors)" << std::endl;
}

void GPUComputeEngine::execute_tensor_batch() {
    process_all();
}

// -- Accessors ----------------------------------------------------------------

GPUQueue&      GPUComputeEngine::queue()        { return queue_; }
GPUMemoryPool& GPUComputeEngine::memory_pool()  { return *pool_; }

std::size_t GPUComputeEngine::commands_processed() const { return commands_processed_; }
std::size_t GPUComputeEngine::batches_processed() const { return batches_processed_; }
std::size_t GPUComputeEngine::tensors_in_batches() const { return tensors_in_batches_; }
bool        GPUComputeEngine::is_initialized()   const { return initialized_; }

} // namespace liz