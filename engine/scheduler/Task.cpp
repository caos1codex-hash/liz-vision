#include "engine/scheduler/Task.h"
#include "engine/core/Logger.h"

#include <atomic>
#include <sstream>

namespace liz {

// ── Auto-incrementing ID ─────────────────────────────────────────────────────
TaskId Task::next_id() {
    static std::atomic<TaskId> counter{1};
    return counter.fetch_add(1);
}

// ── Constructor ──────────────────────────────────────────────────────────────
Task::Task(std::string name, ExecuteFn fn)
    : id_(next_id())
    , name_(std::move(name))
    , fn_(std::move(fn))
    , status_(TaskStatus::Pending)
{
    std::ostringstream oss;
    oss << "Task created: #" << id_ << " [" << name_ << "]";
    LIZ_TRACE(oss.str());
}

// ── Accessors ────────────────────────────────────────────────────────────────
TaskId Task::id() const { return id_; }

const std::string& Task::name() const { return name_; }

TaskStatus Task::status() const { return status_; }

// ── Execution ────────────────────────────────────────────────────────────────
bool Task::execute() {
    if (status_ != TaskStatus::Pending) {
        std::ostringstream oss;
        oss << "Task #" << id_ << " [" << name_
            << "] cannot execute: status is "
            << task_status_to_string(status_);
        LIZ_WARN(oss.str());
        return false;
    }

    if (!fn_) {
        std::ostringstream oss;
        oss << "Task #" << id_ << " [" << name_ << "] has no payload";
        LIZ_ERROR(oss.str());
        status_ = TaskStatus::Failed;
        return false;
    }

    status_ = TaskStatus::Running;

    try {
        bool ok = fn_();
        status_ = ok ? TaskStatus::Completed : TaskStatus::Failed;
        return ok;
    } catch (...) {
        status_ = TaskStatus::Failed;
        {
            std::ostringstream oss;
            oss << "Task #" << id_ << " [" << name_ << "] threw an exception";
            LIZ_ERROR(oss.str());
        }
        return false;
    }
}

void Task::cancel() {
    status_ = TaskStatus::Failed;
    std::ostringstream oss;
    oss << "Task #" << id_ << " [" << name_ << "] cancelled";
    LIZ_WARN(oss.str());
}

// ── Free function ────────────────────────────────────────────────────────────
const char* task_status_to_string(TaskStatus status) {
    switch (status) {
        case TaskStatus::Pending:   return "Pending";
        case TaskStatus::Running:   return "Running";
        case TaskStatus::Completed: return "Completed";
        case TaskStatus::Failed:    return "Failed";
    }
    return "Unknown";
}

} // namespace liz