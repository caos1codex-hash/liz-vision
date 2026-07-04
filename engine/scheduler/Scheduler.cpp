#include "engine/scheduler/Scheduler.h"
#include "engine/core/Logger.h"

#include <sstream>

namespace liz {

// ── Task submission ──────────────────────────────────────────────────────────
TaskId Scheduler::submit(std::unique_ptr<Task> task) {
    auto id = task->id();
    queue_.push(std::move(task));

    std::ostringstream oss;
    oss << "Scheduler: task #" << id << " enqueued";
    LIZ_DEBUG(oss.str());

    return id;
}

TaskId Scheduler::submit(std::string name, Task::ExecuteFn fn) {
    auto task = std::make_unique<Task>(std::move(name), std::move(fn));
    return submit(std::move(task));
}

// ── Execution ────────────────────────────────────────────────────────────────
bool Scheduler::run_next() {
    auto task = queue_.pop();
    if (!task) {
        return false;
    }

    auto id = task->id();
    const auto& name = task->name();

    {
        std::ostringstream oss;
        oss << "Scheduler: executing task #" << id << " [" << name << "]";
        LIZ_INFO(oss.str());
    }

    bool ok = task->execute();

    {
        std::ostringstream oss;
        oss << "Scheduler: task #" << id << " ["
            << name << "] -> "
            << task_status_to_string(task->status());
        LIZ_INFO(oss.str());
    }

    return true;
}

std::size_t Scheduler::run_all() {
    std::size_t count = 0;
    while (run_next()) {
        ++count;
    }
    return count;
}

// ── Query ────────────────────────────────────────────────────────────────────
std::size_t Scheduler::pending_count() const {
    return queue_.size();
}

bool Scheduler::has_pending() const {
    return !queue_.empty();
}

} // namespace liz