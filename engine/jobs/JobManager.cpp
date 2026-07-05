#include "engine/jobs/JobManager.h"
#include "engine/jobs/JobTypes.h"
#include "engine/scheduler/Scheduler.h"
#include "engine/scheduler/Task.h"
#include "engine/events/EventBus.h"
#include "engine/events/Event.h"
#include "engine/events/EventType.h"
#include "engine/core/Logger.h"

#include <algorithm>
#include <chrono>
#include <sstream>
#include <thread>

namespace liz {

// ── Constructor / Destructor ──────────────────────────────────────────────

JobManager::JobManager() = default;

JobManager::~JobManager() {
    if (scheduler_) {
        shutdown();
    }
}

// ── Lifecycle ────────────────────────────────────────────────────────────

void JobManager::initialize(Scheduler* scheduler) {
    scheduler_ = scheduler;
    LIZ_INFO("JobManager: initialized (connected to Scheduler)");
}

void JobManager::shutdown() {
    clear();
    scheduler_ = nullptr;
    event_bus_ = nullptr;
    LIZ_INFO("JobManager: shutdown");
}

// ── EventBus ───────────────────────────────────────────────────────────

void JobManager::set_event_bus(EventBus* bus) {
    event_bus_ = bus;
    LIZ_INFO("JobManager: connected to EventBus");
}

// ── Job submission ──────────────────────────────────────────────────────

std::string JobManager::submit(std::unique_ptr<Job> job) {
    if (!job) return "";

    const std::string uuid = job->uuid();

    // Set state to Queued.
    job->set_state(JobState::Queued);

    // Publish JobCreated event.
    publish_event(0, uuid, "Job created: " + job->name());

    // Publish JobQueued event.
    publish_event(1, uuid, "Job queued: " + job->name());

    // Enqueue.
    queue_.push(std::move(job));

    ++stats_.jobs_created;

    {
        std::ostringstream oss;
        oss << "JobManager: job submitted — uuid=" << uuid
            << ", queue_size=" << queue_.size();
        LIZ_INFO(oss.str());
    }

    return uuid;
}

std::string JobManager::submit(std::string name, JobType type, JobPriority priority) {
    auto job = std::make_unique<Job>(std::move(name), type, priority);
    return submit(std::move(job));
}

// ── Execution ───────────────────────────────────────────────────────────

bool JobManager::cancel(const std::string& uuid) {
    // Can only cancel queued jobs (not running or completed).
    // For simplicity, scan the queue to find the job.
    // Since JobQueue uses std::queue internally without random access,
    // we cannot efficiently remove from the middle.
    // Instead, we check if the job exists in the queue and mark it conceptually.
    // For this sprint, we cancel by checking the front of the queue.

    // Check if we can find it in history first (already processed).
    for (const auto& j : history_) {
        if (j->uuid() == uuid) {
            return false; // Already processed.
        }
    }

    // The job might be in the queue but not at the front.
    // For this sprint's FIFO implementation, cancellation is best-effort.
    // We log a warning if we cannot cancel.
    {
        std::ostringstream oss;
        oss << "JobManager: cancel requested — uuid=" << uuid
            << " (best-effort, FIFO queue)";
        LIZ_WARN(oss.str());
    }
    return false;
}

bool JobManager::process_next() {
    if (!scheduler_) {
        LIZ_WARN("JobManager: no scheduler connected");
        return false;
    }

    auto job = queue_.pop();
    if (!job) return false;

    const std::string uuid = job->uuid();
    const std::string name = job->name();

    // Transition to Running.
    job->set_state(JobState::Running);

    // Record start time.
    auto start_time = std::chrono::steady_clock::now();
    auto start_ms = static_cast<std::uint64_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            start_time.time_since_epoch()).count());
    job->set_start_time(start_ms);

    ++stats_.jobs_running;

    // Publish JobStarted event.
    publish_event(2, uuid, "Job started: " + name);

    {
        std::ostringstream oss;
        oss << "JobManager: processing job — " << job->to_string();
        LIZ_INFO(oss.str());
    }

    // Convert the Job into a Scheduler Task.
    // The task closure captures the job pointer and updates state on completion.
    // The job is owned by the closure and will be moved to history after.
    auto job_ptr = job.get();

    // Create the task with a lambda that simulates work.
    auto task_fn = [this, job_ptr, uuid, name]() -> bool {
        // Simulate work with a small sleep (real jobs would do actual work).
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        // Record timing.
        auto end_time = std::chrono::steady_clock::now();
        auto end_ms = static_cast<std::uint64_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                end_time.time_since_epoch()).count());
        auto duration = static_cast<double>(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                end_time - std::chrono::steady_clock::time_point{
                    std::chrono::milliseconds(job_ptr->start_time())}
            ).count());

        job_ptr->set_end_time(end_ms);
        job_ptr->set_duration_ms(duration);
        job_ptr->set_progress(100);

        // Determine success/failure — simulated 100% success for now.
        bool success = true;
        if (success) {
            job_ptr->set_state(JobState::Completed);
            publish_event(3, uuid, "Job completed: " + name);
        } else {
            job_ptr->set_state(JobState::Failed);
            job_ptr->set_error_message("Simulated failure");
            publish_event(4, uuid, "Job failed: " + name);
        }

        return success;
    };

    // Submit the task to the Scheduler.
    auto task = std::make_unique<liz::Task>(name, task_fn);

    // Execute the task immediately (synchronous, single-threaded).
    bool task_success = task->execute();

    // Record end time and duration from the job itself (set in the lambda).
    double exec_duration = job->duration_ms();
    stats_.job_execution_time_ms += exec_duration;
    --stats_.jobs_running;

    if (task_success) {
        ++stats_.jobs_completed;
    } else {
        ++stats_.jobs_failed;
    }

    // Move job to history.
    history_.push_back(std::move(job));

    {
        std::ostringstream oss;
        oss << "JobManager: job finished — uuid=" << uuid
            << ", state=" << job_state_to_string(history_.back()->state())
            << ", duration=" << std::fixed << std::setprecision(1) << exec_duration << "ms";
        LIZ_INFO(oss.str());
    }

    return true;
}

std::size_t JobManager::process_all() {
    std::size_t count = 0;
    while (process_next()) {
        ++count;
    }
    return count;
}

// ── Queries ─────────────────────────────────────────────────────────────

std::vector<const Job*> JobManager::running_jobs() const {
    // In this synchronous implementation, jobs are not "running" between calls.
    // Running count is tracked in stats_ during process_next execution.
    std::vector<const Job*> result;
    return result;
}

std::vector<const Job*> JobManager::completed_jobs() const {
    std::vector<const Job*> result;
    for (const auto& j : history_) {
        if (j->state() == JobState::Completed) {
            result.push_back(j.get());
        }
    }
    return result;
}

std::vector<const Job*> JobManager::failed_jobs() const {
    std::vector<const Job*> result;
    for (const auto& j : history_) {
        if (j->state() == JobState::Failed || j->state() == JobState::Cancelled) {
            result.push_back(j.get());
        }
    }
    return result;
}

JobStatistics JobManager::statistics() const {
    return stats_;
}

// ── Reset ────────────────────────────────────────────────────────────────

void JobManager::clear() {
    queue_.clear();
    history_.clear();
    stats_ = JobStatistics{};
    LIZ_INFO("JobManager: cleared");
}

// ── Event publishing ────────────────────────────────────────────────────

void JobManager::publish_event(int event_code, const std::string& uuid, const std::string& message) {
    if (!event_bus_) return;

    EventType type;
    switch (event_code) {
        case 0: type = EventType::JobCreated;    break;
        case 1: type = EventType::JobQueued;     break;
        case 2: type = EventType::JobStarted;    break;
        case 3: type = EventType::JobCompleted; break;
        case 4: type = EventType::JobFailed;    break;
        case 5: type = EventType::JobCancelled; break;
        default: type = EventType::Custom;       break;
    }

    Event event(type, "JobManager", message);
    event_bus_->publish(event);
}

// ── Internal helpers ─────────────────────────────────────────────────────

Job* JobManager::find_job_by_uuid(const std::string& uuid) {
    for (auto& j : history_) {
        if (j->uuid() == uuid) return j.get();
    }
    return nullptr;
}

} // namespace liz
