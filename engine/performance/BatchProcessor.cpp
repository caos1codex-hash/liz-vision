#include "engine/performance/BatchProcessor.h"
#include "engine/core/Logger.h"

#include <chrono>
#include <sstream>

namespace liz {

// -- Constructor --------------------------------------------------------------
BatchProcessor::BatchProcessor(TaskExecutor& executor, std::size_t batch_size)
    : executor_(executor), batch_size_(batch_size) {
    std::ostringstream oss;
    oss << "BatchProcessor: created with batch_size=" << batch_size;
    LIZ_INFO(oss.str());
}

// -- Async processing ---------------------------------------------------------
void BatchProcessor::process(
    const std::vector<VideoFrame>& frames,
    std::function<bool(std::size_t, const std::vector<VideoFrame>&)> process_fn)
{
    stats_ = BatchStats{};
    stats_.total_frames = frames.size();
    stats_.batch_size   = batch_size_;

    if (frames.empty()) {
        LIZ_WARN("BatchProcessor: no frames to process");
        return;
    }

    auto total_start = std::chrono::steady_clock::now();

    std::size_t num_batches = (frames.size() + batch_size_ - 1) / batch_size_;
    stats_.total_batches = num_batches;

    {
        std::ostringstream oss;
        oss << "BatchProcessor: splitting " << frames.size()
            << " frames into " << num_batches
            << " batch(es) of up to " << batch_size_;
        LIZ_INFO(oss.str());
    }

    for (std::size_t b = 0; b < num_batches; ++b) {
        auto start = static_cast<std::size_t>(b) * batch_size_;
        auto end   = std::min(start + batch_size_, frames.size());

        std::vector<VideoFrame> batch(
            frames.begin() + static_cast<std::ptrdiff_t>(start),
            frames.begin() + static_cast<std::ptrdiff_t>(end));

        auto batch_idx = b;
        auto batch_sz  = batch.size();

        executor_.submit(
            "Batch #" + std::to_string(b),
            [this, batch_idx, batch_sz, batch = std::move(batch),
             &process_fn]() -> bool {

                std::ostringstream oss;
                oss << "BatchProcessor: Batch #" << batch_idx
                    << " processing " << batch_sz << " frame(s)";
                LIZ_INFO(oss.str());

                auto bstart = std::chrono::steady_clock::now();
                bool ok = process_fn(batch_idx, batch);
                auto bend = std::chrono::steady_clock::now();

                double belapsed =
                    std::chrono::duration<double, std::milli>(bend - bstart).count();

                std::ostringstream oss2;
                oss2 << "BatchProcessor: Batch #" << batch_idx
                     << " " << (ok ? "completed" : "FAILED")
                     << " in " << belapsed << " ms";
                LIZ_INFO(oss2.str());

                return ok;
            }
        );
    }

    // Wait for all batch tasks to complete.
    executor_.wait_all();

    auto total_end = std::chrono::steady_clock::now();
    stats_.total_time_ms =
        std::chrono::duration<double, std::milli>(total_end - total_start).count();
    stats_.avg_batch_ms = (num_batches > 0)
        ? stats_.total_time_ms / static_cast<double>(num_batches)
        : 0.0;
    stats_.frames_processed = frames.size();

    {
        std::ostringstream oss;
        oss << "BatchProcessor complete: "
            << stats_.frames_processed << " frames, "
            << stats_.total_batches << " batches, "
            << stats_.total_time_ms << " ms total, "
            << stats_.avg_batch_ms << " ms avg/batch";
        LIZ_INFO(oss.str());
    }
}

// -- Sync processing -----------------------------------------------------------
void BatchProcessor::process_sync(
    const std::vector<VideoFrame>& frames,
    std::function<void(std::size_t, const std::vector<VideoFrame>&)> process_fn)
{
    stats_ = BatchStats{};
    stats_.total_frames = frames.size();
    stats_.batch_size   = batch_size_;

    if (frames.empty()) return;

    auto total_start = std::chrono::steady_clock::now();

    std::size_t num_batches = (frames.size() + batch_size_ - 1) / batch_size_;
    stats_.total_batches = num_batches;

    {
        std::ostringstream oss;
        oss << "BatchProcessor [sync]: splitting " << frames.size()
            << " frames into " << num_batches
            << " batch(es) of up to " << batch_size_;
        LIZ_INFO(oss.str());
    }

    for (std::size_t b = 0; b < num_batches; ++b) {
        auto start = static_cast<std::size_t>(b) * batch_size_;
        auto end   = std::min(start + batch_size_, frames.size());

        std::vector<VideoFrame> batch(
            frames.begin() + static_cast<std::ptrdiff_t>(start),
            frames.begin() + static_cast<std::ptrdiff_t>(end));

        auto bstart = std::chrono::steady_clock::now();
        process_fn(b, batch);
        auto bend = std::chrono::steady_clock::now();

        double belapsed =
            std::chrono::duration<double, std::milli>(bend - bstart).count();

        std::ostringstream oss;
        oss << "BatchProcessor [sync]: Batch #" << b
            << " (" << batch.size() << " frames) done in "
            << belapsed << " ms";
        LIZ_INFO(oss.str());
    }

    auto total_end = std::chrono::steady_clock::now();
    stats_.total_time_ms =
        std::chrono::duration<double, std::milli>(total_end - total_start).count();
    stats_.avg_batch_ms = (num_batches > 0)
        ? stats_.total_time_ms / static_cast<double>(num_batches)
        : 0.0;
    stats_.frames_processed = frames.size();
}

// -- Query ---------------------------------------------------------------------
std::size_t      BatchProcessor::batch_size() const { return batch_size_; }
const BatchStats& BatchProcessor::last_stats() const { return stats_; }

} // namespace liz