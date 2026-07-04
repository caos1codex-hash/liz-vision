#pragma once

#include "engine/performance/ThreadPool.h"

#include <atomic>
#include <chrono>
#include <cstddef>
#include <functional>
#include <vector>

namespace liz {

/// Statistics from the task executor.
struct TaskExecutorStats {
    std::size_t tasks_submitted  = 0;
    std::size_t tasks_completed  = 0;
    std::size_t tasks_failed     = 0;
    double      total_time_ms    = 0.0;
    double      avg_time_ms      = 0.0;
};

/// Bridges the existing Task/Scheduler system with the ThreadPool.
///
/// Accepts named work items, wraps them with timing, and dispatches
/// them to a ThreadPool for parallel execution.
///
/// The TaskExecutor does NOT replace the original single-threaded
/// Scheduler — it is an alternative path for high-throughput scenarios.
///
/// Usage:
///   TaskExecutor executor(pool);
///   executor.submit("my_work", []{ ... });
///   executor.wait_all();
///   auto s = executor.stats();
class TaskExecutor {
public:
    /// Construct an executor backed by the given thread pool.
    explicit TaskExecutor(ThreadPool& pool);

    // -- Submission -------------------------------------------------------------
    /// Submit a named unit of work for async execution on the pool.
    void submit(std::string name, std::function<bool()> work);

    /// Submit a fire-and-forget task (void return).
    void submit_void(std::string name, std::function<void()> work);

    // -- Synchronization --------------------------------------------------------
    /// Block until all submitted tasks have finished executing.
    /// Call this after submitting a batch to ensure completion.
    void wait_all();

    // -- Query ------------------------------------------------------------------
    /// Snapshot of executor statistics.
    TaskExecutorStats stats() const;

    /// Reset all counters (for a new benchmark run).
    void reset_stats();

private:
    ThreadPool&                  pool_;
    std::atomic<std::size_t>     in_flight_{0};
    mutable std::mutex           stats_mutex_;
    std::condition_variable      cv_done_;

    std::size_t                  submitted_  = 0;
    std::size_t                  completed_  = 0;
    std::size_t                  failed_     = 0;
    double                       total_ms_   = 0.0;
};

} // namespace liz