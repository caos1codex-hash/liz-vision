#pragma once

#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace liz {

/// Statistics from the thread pool.
struct ThreadPoolStats {
    std::size_t tasks_submitted   = 0;
    std::size_t tasks_completed   = 0;
    std::size_t tasks_failed      = 0;
    std::size_t active_workers    = 0;
    std::size_t total_workers     = 0;
    std::size_t queue_depth       = 0;
};

/// A fixed-size thread pool with a concurrent task queue.
///
/// Workers pull callables from an internal queue and execute them.
/// Supports graceful shutdown: stops accepting new work and waits
/// for in-flight tasks to finish.
///
/// Usage:
///   ThreadPool pool(4);
///   pool.start();
///   pool.submit([]{ /* work */ });
///   pool.shutdown();   // blocks until all tasks done
class ThreadPool {
public:
    /// Construct a pool with the given number of worker threads.
    /// Does NOT start the workers — call start() explicitly.
    explicit ThreadPool(std::size_t num_threads);

    /// Destructor calls shutdown() if not already shut down.
    ~ThreadPool();

    // Non-copyable, non-movable.
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    // -- Lifecycle ---------------------------------------------------------------
    /// Spawn all worker threads.  Returns true on success.
    bool start();

    /// Graceful shutdown: stop accepting tasks, finish in-flight work.
    void shutdown();

    /// True after a successful start(), false after shutdown().
    bool is_running() const;

    // -- Task submission ---------------------------------------------------------
    /// Enqueue a callable for execution by the next available worker.
    void submit(std::function<void()> task);

    // -- Query -------------------------------------------------------------------
    /// Current number of tasks waiting in the queue.
    std::size_t queue_depth() const;

    /// Number of workers currently executing a task.
    std::size_t active_workers() const;

    /// Total number of workers in the pool.
    std::size_t worker_count() const;

    /// Snapshot of pool statistics.
    ThreadPoolStats stats() const;

private:
    /// Worker loop — runs until stop_ is set.
    void worker_loop(std::size_t worker_id);

    std::size_t                        num_threads_;
    std::vector<std::thread>           workers_;
    std::queue<std::function<void()>>  task_queue_;

    mutable std::mutex                 mutex_;
    std::condition_variable            cv_task_;     // signaled when a task is enqueued
    std::condition_variable            cv_done_;     // signaled when a task finishes

    std::atomic<bool>                  stop_{false};
    std::atomic<std::size_t>           active_count_{0};
    std::size_t                        submitted_{0};
    std::size_t                        completed_{0};
    std::size_t                        failed_{0};
};

} // namespace liz