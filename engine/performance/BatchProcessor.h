#pragma once

#include "engine/performance/ThreadPool.h"
#include "engine/performance/TaskExecutor.h"
#include "engine/video/VideoFrame.h"

#include <cstddef>
#include <functional>
#include <vector>

namespace liz {

/// Statistics from a batch processor run.
struct BatchStats {
    std::size_t total_frames     = 0;
    std::size_t total_batches    = 0;
    std::size_t batch_size       = 0;
    std::size_t frames_processed = 0;
    double      total_time_ms    = 0.0;
    double      avg_batch_ms     = 0.0;
};

/// Groups VideoFrames into fixed-size batches and processes each batch
/// via a callback (typically routed through the InferenceEngine).
///
/// This simulates the GPU batching optimization pattern where
/// multiple frames are processed together for better throughput.
///
/// Usage:
///   BatchProcessor bp(executor, 4);
///   bp.process(frames, [](const auto& batch) { ... });
///   auto s = bp.last_stats();
class BatchProcessor {
public:
    /// Construct with a TaskExecutor (for async dispatch) and batch size.
    BatchProcessor(TaskExecutor& executor, std::size_t batch_size);

    // -- Processing -------------------------------------------------------------
    /// Split frames into batches and dispatch each batch to the executor.
    /// @param frames     Input frames to process.
    /// @param process_fn Callback invoked for each batch (batch_index, batch).
    ///                   Should return true on success.
    void process(const std::vector<VideoFrame>& frames,
                 std::function<bool(std::size_t batch_index,
                                    const std::vector<VideoFrame>& batch)> process_fn);

    /// Synchronous (blocking) batch processing — no thread pool.
    void process_sync(const std::vector<VideoFrame>& frames,
                      std::function<void(std::size_t batch_index,
                                         const std::vector<VideoFrame>& batch)> process_fn);

    // -- Query ------------------------------------------------------------------
    std::size_t     batch_size() const;
    const BatchStats& last_stats() const;

private:
    TaskExecutor&  executor_;
    std::size_t    batch_size_;
    BatchStats     stats_;
};

} // namespace liz