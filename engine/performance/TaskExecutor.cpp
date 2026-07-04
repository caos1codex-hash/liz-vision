#include "engine/performance/TaskExecutor.h"
#include "engine/core/Logger.h"

#include <sstream>

namespace liz {

// -- Constructor ---------------------------------------------------------------
TaskExecutor::TaskExecutor(ThreadPool& pool)
    : pool_(pool) {}

// -- Submission ---------------------------------------------------------------
void TaskExecutor::submit(std::string name, std::function<bool()> work) {
    ++in_flight_;
    {
        std::lock_guard<std::mutex> lock(stats_mutex_);
        ++submitted_;
    }

    pool_.submit([this, name = std::move(name), work = std::move(work)]() mutable {
        auto start = std::chrono::steady_clock::now();
        bool ok = false;

        try {
            ok = work();
        } catch (...) {
            ok = false;
        }

        auto end = std::chrono::steady_clock::now();
        double elapsed =
            std::chrono::duration<double, std::milli>(end - start).count();

        {
            std::lock_guard<std::mutex> lock(stats_mutex_);
            if (ok) {
                ++completed_;
            } else {
                ++failed_;
            }
            total_ms_ += elapsed;
        }

        std::ostringstream oss;
        oss << "TaskExecutor: '" << name << "' "
            << (ok ? "completed" : "FAILED")
            << " in " << elapsed << " ms";
        LIZ_DEBUG(oss.str());

        --in_flight_;
        cv_done_.notify_all();
    });
}

void TaskExecutor::submit_void(std::string name, std::function<void()> work) {
    submit(std::move(name), [w = std::move(work)]() -> bool {
        w();
        return true;
    });
}

// -- Synchronization -----------------------------------------------------------
void TaskExecutor::wait_all() {
    std::unique_lock<std::mutex> lock(stats_mutex_);
    cv_done_.wait(lock, [this] {
        return in_flight_.load() == 0;
    });
}

// -- Query ---------------------------------------------------------------------
TaskExecutorStats TaskExecutor::stats() const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    TaskExecutorStats s;
    s.tasks_submitted = submitted_;
    s.tasks_completed = completed_;
    s.tasks_failed    = failed_;
    s.total_time_ms   = total_ms_;
    s.avg_time_ms     = (completed_ > 0)
                            ? total_ms_ / static_cast<double>(completed_)
                            : 0.0;
    return s;
}

void TaskExecutor::reset_stats() {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    submitted_ = 0;
    completed_ = 0;
    failed_    = 0;
    total_ms_   = 0.0;
}

} // namespace liz