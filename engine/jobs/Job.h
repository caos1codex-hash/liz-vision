#pragma once

#include "engine/jobs/JobTypes.h"

#include <chrono>
#include <cstdint>
#include <string>
#include <utility>

namespace liz {

/// A Job is a high-level unit of work managed by the Job System.
///
/// Jobs are submitted to the JobManager, which converts each job into
/// a Scheduler Task for execution.  Jobs carry metadata (type, priority,
/// state, progress, timing) that is tracked throughout their lifecycle.
///
/// Non-copyable, movable.
class Job {
public:
    /// Construct a job with the given name, type, and priority.
    Job(std::string name, JobType type, JobPriority priority = JobPriority::Normal);

    // Non-copyable.
    Job(const Job&) = delete;
    Job& operator=(const Job&) = delete;

    // Movable.
    Job(Job&&) noexcept = default;
    Job& operator=(Job&&) noexcept = default;

    ~Job() = default;

    // ── Accessors ────────────────────────────────────────────────────────

    /// Unique identifier (8-hex random string).
    const std::string& uuid() const;

    /// Job name.
    const std::string& name() const;

    /// Job type category.
    JobType type() const;

    /// Job priority level.
    JobPriority priority() const;

    /// Current state in the lifecycle.
    JobState state() const;

    /// Execution progress (0-100).
    std::uint8_t progress() const;

    /// Timestamp when the job was created.
    std::uint64_t creation_time() const;

    /// Timestamp when the job started running.
    std::uint64_t start_time() const;

    /// Timestamp when the job finished (completed, failed, or cancelled).
    std::uint64_t end_time() const;

    /// Execution duration in milliseconds.
    double duration_ms() const;

    /// Error message (empty if no error).
    const std::string& error_message() const;

    /// Whether the job can be cancelled.
    bool is_cancelable() const;

    /// User data pointer (opaque, for application use).
    void* user_data() const;

    // ── Mutators ────────────────────────────────────────────────────────

    void set_state(JobState state);
    void set_progress(std::uint8_t progress);
    void set_start_time(std::uint64_t time);
    void set_end_time(std::uint64_t time);
    void set_duration_ms(double duration);
    void set_error_message(const std::string& message);
    void set_cancelable(bool cancelable);
    void set_user_data(void* data);

    // ── Utility ─────────────────────────────────────────────────────────

    /// Generate a formatted summary string.
    std::string to_string() const;

private:
    /// Generate a random 8-hex UUID.
    static std::string generate_uuid();

    std::string  uuid_;
    std::string  name_;
    JobType      type_;
    JobPriority  priority_;
    JobState     state_        = JobState::Created;
    std::uint8_t progress_     = 0;
    std::uint64_t creation_time_ = 0;
    std::uint64_t start_time_   = 0;
    std::uint64_t end_time_     = 0;
    double       duration_ms_  = 0.0;
    std::string  error_message_;
    bool         cancelable_   = true;
    void*        user_data_    = nullptr;
};

} // namespace liz
