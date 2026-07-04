#include "engine/performance/PerformanceManager.h"
#include "engine/core/Logger.h"

#include <algorithm>
#include <numeric>
#include <sstream>

namespace liz {

// -- Frame tracking -----------------------------------------------------------
void PerformanceManager::record_frame_produced() {
    std::lock_guard<std::mutex> lock(mutex_);
    ++frames_produced_;
}

void PerformanceManager::record_frame_consumed() {
    std::lock_guard<std::mutex> lock(mutex_);
    ++frames_consumed_;
}

void PerformanceManager::record_latency(std::string label, double time_ms) {
    std::lock_guard<std::mutex> lock(mutex_);
    latency_samples_.push_back({std::move(label), time_ms});
}

void PerformanceManager::record_batch_time(double time_ms) {
    std::lock_guard<std::mutex> lock(mutex_);
    batch_times_.push_back(time_ms);
}

// -- Metrics ------------------------------------------------------------------
double PerformanceManager::effective_fps() const {
    std::lock_guard<std::mutex> lock(mutex_);
    if (frames_consumed_ < 2) return 0.0;

    auto now  = std::chrono::steady_clock::now();
    auto elapsed_s =
        std::chrono::duration<double>(now - start_time_).count();

    if (elapsed_s <= 0.0) return 0.0;
    return static_cast<double>(frames_consumed_) / elapsed_s;
}

std::size_t PerformanceManager::frames_produced() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return frames_produced_;
}

std::size_t PerformanceManager::frames_consumed() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return frames_consumed_;
}

double PerformanceManager::avg_frame_latency_ms() const {
    std::lock_guard<std::mutex> lock(mutex_);
    if (latency_samples_.empty()) return 0.0;

    double sum = 0.0;
    for (const auto& s : latency_samples_) {
        sum += s.time_ms;
    }
    return sum / static_cast<double>(latency_samples_.size());
}

double PerformanceManager::avg_batch_time_ms() const {
    std::lock_guard<std::mutex> lock(mutex_);
    if (batch_times_.empty()) return 0.0;

    double sum = std::accumulate(batch_times_.begin(), batch_times_.end(), 0.0);
    return sum / static_cast<double>(batch_times_.size());
}

std::size_t PerformanceManager::batch_count() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return batch_times_.size();
}

// -- Pipeline health -----------------------------------------------------------
bool PerformanceManager::is_healthy() const {
    std::lock_guard<std::mutex> lock(mutex_);
    // Pipeline is healthy if producer-consumer gap is < 50 frames.
    std::ptrdiff_t gap = static_cast<std::ptrdiff_t>(frames_produced_)
                       - static_cast<std::ptrdiff_t>(frames_consumed_);
    return gap < 50;
}

std::string PerformanceManager::pipeline_status() const {
    std::lock_guard<std::mutex> lock(mutex_);

    auto now = std::chrono::steady_clock::now();
    auto elapsed_s =
        std::chrono::duration<double>(now - start_time_).count();

    double fps = (elapsed_s > 0.0 && frames_consumed_ > 0)
        ? static_cast<double>(frames_consumed_) / elapsed_s
        : 0.0;

    std::ptrdiff_t gap = static_cast<std::ptrdiff_t>(frames_produced_)
                       - static_cast<std::ptrdiff_t>(frames_consumed_);

    double avg_lat = 0.0;
    if (!latency_samples_.empty()) {
        double sum = 0.0;
        for (const auto& s : latency_samples_) sum += s.time_ms;
        avg_lat = sum / static_cast<double>(latency_samples_.size());
    }

    std::ostringstream oss;
    oss << "produced=" << frames_produced_
        << " consumed=" << frames_consumed_
        << " gap=" << gap
        << " fps=" << fps
        << " avg_latency=" << avg_lat << "ms"
        << " batches=" << batch_times_.size()
        << " health=" << (gap < 50 ? "OK" : "BACKPRESSURE");
    return oss.str();
}

// -- Reporting ----------------------------------------------------------------
void PerformanceManager::log_summary() const {
    auto status = pipeline_status();

    LIZ_INFO("========== Performance Summary ==========");
    LIZ_INFO(std::string("  " + status));
    LIZ_INFO(std::string("  effective_fps=" +
              std::to_string(effective_fps())));
    LIZ_INFO(std::string("  avg_frame_latency=" +
              std::to_string(avg_frame_latency_ms()) + " ms"));
    LIZ_INFO(std::string("  avg_batch_time=" +
              std::to_string(avg_batch_time_ms()) + " ms"));
    LIZ_INFO(std::string("  total_batches=" +
              std::to_string(batch_count())));
    LIZ_INFO("=========================================");
}

// -- Reset --------------------------------------------------------------------
void PerformanceManager::reset() {
    std::lock_guard<std::mutex> lock(mutex_);
    frames_produced_  = 0;
    frames_consumed_  = 0;
    latency_samples_.clear();
    batch_times_.clear();
    start_time_ = std::chrono::steady_clock::now();
}

} // namespace liz