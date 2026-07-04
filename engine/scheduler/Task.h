#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <string_view>

namespace liz {

/// Status of a Task through its lifecycle.
enum class TaskStatus {
    Pending,    ///< Created but not yet executed
    Running,    ///< Currently being executed
    Completed,  ///< Finished successfully
    Failed      ///< Finished with an error
};

/// Converts a TaskStatus to a human-readable string.
const char* task_status_to_string(TaskStatus status);

/// Unique identifier type for tasks.
using TaskId = std::uint64_t;

/// A Task is the fundamental unit of work in LIZ Vision.
///
/// Each task carries a name, a callable payload, and tracks its own
/// execution status.  The Scheduler consumes tasks via the TaskQueue.
class Task {
public:
    /// The callable signature every task must satisfy.
    /// Returns true on success, false on failure.
    using ExecuteFn = std::function<bool()>;

    /// Construct a named task with a given workload.
    Task(std::string name, ExecuteFn fn);

    /// Non-copyable (unique ownership by the queue).
    Task(const Task&) = delete;
    Task& operator=(const Task&) = delete;

    /// Movable.
    Task(Task&&) noexcept = default;
    Task& operator=(Task&&) noexcept = default;

    // ── Accessors ────────────────────────────────────────────────────────────
    TaskId           id() const;
    const std::string& name() const;
    TaskStatus       status() const;

    // ── Execution ────────────────────────────────────────────────────────────
    /// Run the payload.  Sets status to Completed or Failed.
    /// Returns true if the task succeeded.
    bool execute();

    /// Mark the task as cancelled (sets Failed without running).
    void cancel();

private:
    static TaskId next_id();

    TaskId         id_;
    std::string    name_;
    ExecuteFn      fn_;
    TaskStatus     status_ = TaskStatus::Pending;
};

} // namespace liz