#include "engine/performance/ThreadPool.h"
#include "engine/core/Logger.h"

#include <sstream>

namespace liz {

// -- Constructor / Destructor --------------------------------------------------
ThreadPool::ThreadPool(std::size_t num_threads)
    : num_threads_(num_threads) {}

ThreadPool::~ThreadPool() {
    if (is_running()) {
        shutdown();
    }
}

// -- Lifecycle ------------------------------------------------------------------
bool ThreadPool::start() {
    if (is_running()) {
        LIZ_WARN("ThreadPool: already running");
        return true;
    }

    workers_.reserve(num_threads_);
    for (std::size_t i = 0; i < num_threads_; ++i) {
        workers_.emplace_back(&ThreadPool::worker_loop, this, i);
    }

    std::ostringstream oss;
    oss << "ThreadPool: started with " << num_threads_ << " worker thread(s)";
    LIZ_INFO(oss.str());

    return true;
}

void ThreadPool::shutdown() {
    if (!is_running() && workers_.empty()) return;

    {
        std::lock_guard<std::mutex> lock(mutex_);
        stop_ = true;
    }
    cv_task_.notify_all();

    for (auto& w : workers_) {
        if (w.joinable()) {
            w.join();
        }
    }
    workers_.clear();

    std::ostringstream oss;
    oss << "ThreadPool: shut down — "
        << completed_ << " completed, "
        << failed_ << " failed, "
        << submitted_ << " total submitted";
    LIZ_INFO(oss.str());
}

bool ThreadPool::is_running() const {
    return !stop_.load() && !workers_.empty();
}

// -- Task submission -----------------------------------------------------------
void ThreadPool::submit(std::function<void()> task) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (stop_) {
            LIZ_WARN("ThreadPool: cannot submit — pool is shutting down");
            return;
        }
        task_queue_.push(std::move(task));
        ++submitted_;
    }
    cv_task_.notify_one();
}

// -- Query ---------------------------------------------------------------------
std::size_t ThreadPool::queue_depth() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return task_queue_.size();
}

std::size_t ThreadPool::active_workers() const {
    return active_count_.load();
}

std::size_t ThreadPool::worker_count() const {
    return num_threads_;
}

ThreadPoolStats ThreadPool::stats() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return {
        submitted_,
        completed_,
        failed_,
        active_count_.load(),
        num_threads_,
        task_queue_.size()
    };
}

// -- Worker loop ---------------------------------------------------------------
void ThreadPool::worker_loop(std::size_t worker_id) {
    while (true) {
        std::function<void()> task;

        {
            std::unique_lock<std::mutex> lock(mutex_);
            cv_task_.wait(lock, [this] {
                return stop_.load() || !task_queue_.empty();
            });

            if (stop_.load() && task_queue_.empty()) {
                return;
            }

            task = std::move(task_queue_.front());
            task_queue_.pop();
        }

        ++active_count_;

        try {
            task();
            {
                std::lock_guard<std::mutex> lock(mutex_);
                ++completed_;
            }
        } catch (...) {
            {
                std::lock_guard<std::mutex> lock(mutex_);
                ++failed_;
            }
        }

        --active_count_;
        cv_done_.notify_all();
    }
}

} // namespace liz