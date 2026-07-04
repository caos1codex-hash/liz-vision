#pragma once

#include "engine/ai/tensor/Tensor.h"
#include "engine/gpu/batch/Batch.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace liz {

/// Statistics from the GPU batch processor.
struct BatchProcessorGPUStats {
    std::size_t tensors_received  = 0;
    std::size_t batches_created   = 0;
    std::size_t max_batch_size    = 0;
    std::size_t total_batches_processed = 0;
};

/// Groups tensors into fixed-size batches and logs the grouping.
///
/// Flow:
///   frames -> Tensor conversion -> BatchProcessorGPU -> vector<Batch>
///   -> GPUComputeEngine (batch mode)
///
/// No GPU dependency internally — pure grouping logic.
class BatchProcessorGPU {
public:
    /// @param batch_size  Maximum tensors per batch (e.g. 2, 4, 8).
    explicit BatchProcessorGPU(std::size_t batch_size = 4);

    /// Convert VideoFrames to Tensors and group into batches.
    /// Each frame becomes a Tensor of type ModelInput.
    std::vector<Batch> process_frames(
        const std::vector<class VideoFrame>& frames);

    /// Group pre-built tensors into batches.
    std::vector<Batch> process_tensors(
        std::vector<Tensor> tensors);

    /// Process frames, then directly submit each batch to a GPUComputeEngine.
    /// Returns the list of batches that were submitted.
    std::vector<Batch> process_and_submit(
        const std::vector<class VideoFrame>& frames,
        class GPUComputeEngine& engine,
        const std::string& model_name,
        const std::string& operation);

    const BatchProcessorGPUStats& stats() const;

    std::size_t batch_size() const;

private:
    std::size_t              batch_size_;
    std::uint64_t            next_batch_id_ = 1;
    BatchProcessorGPUStats   stats_;
};

} // namespace liz