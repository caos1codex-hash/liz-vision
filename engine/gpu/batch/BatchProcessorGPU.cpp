#include "engine/gpu/batch/BatchProcessorGPU.h"
#include "engine/gpu/compute/GPUComputeEngine.h"
#include "engine/video/VideoFrame.h"
#include "engine/core/Logger.h"

#include <algorithm>
#include <chrono>
#include <sstream>

namespace liz {

// -- Constructor --------------------------------------------------------------

BatchProcessorGPU::BatchProcessorGPU(std::size_t batch_size)
    : batch_size_(batch_size) {
    std::ostringstream oss;
    oss << "BatchProcessorGPU: created with batch_size=" << batch_size_;
    LIZ_INFO(oss.str());
}

// -- Process frames -> tensors -> batches ------------------------------------

std::vector<Batch> BatchProcessorGPU::process_frames(
    const std::vector<VideoFrame>& frames)
{
    std::vector<Tensor> tensors;
    tensors.reserve(frames.size());

    for (const auto& f : frames) {
        auto t = Tensor::from_frame_data(f.width(), f.height(), f.data());
        t.set_type(TensorType::ModelInput);
        tensors.push_back(std::move(t));
    }

    stats_.tensors_received += tensors.size();
    return process_tensors(std::move(tensors));
}

// -- Process pre-built tensors -> batches -------------------------------------

std::vector<Batch> BatchProcessorGPU::process_tensors(
    std::vector<Tensor> tensors)
{
    stats_.tensors_received += tensors.size();

    std::vector<Batch> batches;

    for (std::size_t i = 0; i < tensors.size(); i += batch_size_) {
        Batch batch;
        batch.batch_id  = next_batch_id_++;
        batch.timestamp = std::chrono::duration<double>(
            std::chrono::steady_clock::now().time_since_epoch()).count();

        std::size_t end = std::min(i + batch_size_, tensors.size());
        for (std::size_t j = i; j < end; ++j) {
            batch.add_tensor(std::move(tensors[j]));
        }

        stats_.batches_created++;
        stats_.max_batch_size = std::max(stats_.max_batch_size, batch.size());

        std::ostringstream oss;
        oss << "Batch #" << batch.batch_id
            << " created: " << batch.size() << " tensors ("
            << batch.total_bytes() << " bytes)";
        LIZ_INFO(oss.str());

        batches.push_back(std::move(batch));
    }

    return batches;
}

// -- Process + submit to GPU engine -------------------------------------------

std::vector<Batch> BatchProcessorGPU::process_and_submit(
    const std::vector<VideoFrame>& frames,
    GPUComputeEngine& engine,
    const std::string& model_name,
    const std::string& operation)
{
    auto batches = process_frames(frames);

    for (auto& batch : batches) {
        engine.process_batch(batch, model_name, operation);
        ++stats_.total_batches_processed;
    }

    return batches;
}

// -- Accessors ----------------------------------------------------------------

const BatchProcessorGPUStats& BatchProcessorGPU::stats() const { return stats_; }
std::size_t BatchProcessorGPU::batch_size() const { return batch_size_; }

} // namespace liz