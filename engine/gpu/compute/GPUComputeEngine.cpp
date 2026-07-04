#include "engine/gpu/compute/GPUComputeEngine.h"
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

// -- Command processing -------------------------------------------------------

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

// -- Accessors ----------------------------------------------------------------

GPUQueue&      GPUComputeEngine::queue()        { return queue_; }
GPUMemoryPool& GPUComputeEngine::memory_pool()  { return *pool_; }

std::size_t GPUComputeEngine::commands_processed() const { return commands_processed_; }
bool        GPUComputeEngine::is_initialized()   const { return initialized_; }

} // namespace liz