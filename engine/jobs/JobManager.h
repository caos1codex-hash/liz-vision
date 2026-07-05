#pragma once

#include "engine/jobs/Job.h"
#include "engine/jobs/JobQueue.h"
#include "engine/jobs/JobStatistics.h"
#include "engine/jobs/JobTypes.h"

#include <chrono>
#include <cstddef>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace liz {

// Forward declarations — avoids #include of internal headers.
class Scheduler;
class EventBus;
class Event;

/// Central job management system for the LIZ Vision engine.
///
/// The JobManager provides a high-level abstraction for executing long-running
/// operations (render, inference, import, export, cloud, plugins, etc.).
/// It converts each submitted Job into a Scheduler Task for execution.
///
/// Does NOT replace the existing Scheduler — works on top of it.
/// Uses the single Scheduler instance passed during initialization.
class JobManager {
public:
    JobManager();
    ~JobManager();

    // Non-copyable.
    JobManager(const JobManager&) = delete;
    JobManager& operator=(const JobManager&) = delete;

    // ── Lifecycle ───────────────────────────────────────────────────────

    /// Initialize the job manager with a Scheduler instance.
    /// Does NOT take ownership of the scheduler.
    void initialize(Scheduler* scheduler);

    /// Shut down the job manager and release resources.
    void shutdown();

    // ── EventBus ──────────────────────────────────────────────────────────

    /// Connect to an EventBus for event publishing.  Non-owning.
    void set_event_bus(EventBus* bus);

    // ── Job submission ───────────────────────────────────────────────────

    /// Submit a job for execution.  The job is queued and will be executed
    /// when process_next() or process_all() is called.
    /// Returns the job UUID.
    std::string submit(std::unique_ptr<Job> job);

    /// Convenience: create and submit a named job inline.
    /// Returns the job UUID.
    std::string submit(std::string name, JobType type, JobPriority priority = JobPriority::Normal);

    // ── Execution ───────────────────────────────────────────────────────

    /// Cancel a queued job by UUID.  Returns true if cancelled.
    /// Cannot cancel jobs that are already running or completed.
    bool cancel(const std::string& uuid);

    /// Process the next job in the queue (converts to Scheduler task).
    /// Returns true if a job was processed.
    bool process_next();

    /// Process all queued jobs (converts each to a Scheduler task).
    /// Returns the number of jobs processed.
    std::size_t process_all();

    // ── Queries ─────────────────────────────────────────────────────────

    /// Get all currently running jobs.
    std::vector<const Job*> running_jobs() const;

    /// Get all completed jobs.
    std::vector<const Job*> completed_jobs() const;

    /// Get all failed jobs.
    std::vector<const Job*> failed_jobs() const;

    /// Get cumulative job statistics.
    JobStatistics statistics() const;

    // ── Reset ────────────────────────────────────────────────────────────

    /// Clear all jobs, queues, and statistics.
    void clear();

private:
    /// Publish a job lifecycle event via the EventBus.
    void publish_event(int event_code, const std::string& uuid, const std::string& message);

    /// Find a job by UUID in the completed/failed history.
    Job* find_job_by_uuid(const std::string& uuid);

    Scheduler* scheduler_  = nullptr;
    EventBus*  event_bus_  = nullptr;

    JobQueue queue_;

    // History of finished jobs (completed + failed + cancelled).
    std::vector<std::unique_ptr<Job>> history_;

    // Cumulative statistics.
    JobStatistics stats_;
};

} // namespace liz
