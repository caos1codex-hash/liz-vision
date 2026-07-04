#pragma once

#include <chrono>
#include <cstddef>
#include <mutex>
#include <string>
#include <vector>

namespace liz {

/// A single data point in a latency histogram.
struct LatencySample {
    std::string label;
    double      time_ms;
};

/// Central performance monitoring and metrics hub.
///
/// Tracks:
///   - Simulated FPS
///   - Per-frame latency
///   - Batch processing times
///   - Pipeline health / throughput
///
/// Thread-safe: all mutations go through a mutex.
class PerformanceManager {
public:
    PerformanceManager() = default;

    // -- Frame tracking ---------------------------------------------------------
    /// Record that a frame was produced (e.g. decoded).
    void record_frame_produced();

    /// Record that a frame was consumed (e.g. enhanced output).
    void record_frame_consumed();

    /// Record a per-frame latency sample.
    void record_latency(std::string label, double time_ms);

    /// Record a batch processing time.
    void record_batch_time(double time_ms);

    // -- Metrics ----------------------------------------------------------------
    /// Current effective FPS based on consumed frames.
    double effective_fps() const;

    /// Total frames produced.
    std::size_t frames_produced() const;

    /// Total frames consumed.
    std::size_t frames_consumed() const;

    /// Average frame latency in ms (0 if no samples).
    double avg_frame_latency_ms() const;

    /// Average batch time in ms (0 if no batches).
    double avg_batch_time_ms() const;

    /// Total batch count.
    std::size_t batch_count() const;

    // -- Pipeline health --------------------------------------------------------
    /// Check if the pipeline is healthy:
    ///   - producer/consumer gap is not too large (backpressure OK).
    bool is_healthy() const;

    /// Human-readable pipeline status string.
    std::string pipeline_status() const;

    // -- Reporting --------------------------------------------------------------
    /// Log a full performance summary.
    void log_summary() const;

    /// Reset all counters.
    void reset();

private:
    mutable std::mutex           mutex_;

    std::size_t                  frames_produced_ = 0;
    std::size_t                  frames_consumed_ = 0;
    std::vector<LatencySample>   latency_samples_;
    std::vector<double>          batch_times_;

    std::chrono::steady_clock::time_point start_time_ =
        std::chrono::steady_clock::now();
};

} // namespace liz