#pragma once

#include "engine/scheduler/Task.h"

#include <deque>
#include <memory>
#include <optional>

namespace liz {

/// FIFO queue that owns Task objects.
///
/// Tasks are pushed to the back and popped from the front.
/// The Scheduler drains this queue during each tick.
class TaskQueue {
public:
    TaskQueue() = default;

    /// Enqueue a task (takes ownership).
    void push(std::unique_ptr<Task> task);

    /// Pop the front task.  Returns nullptr if empty.
    std::unique_ptr<Task> pop();

    /// Peek at the front task without removing it.
    Task* front();

    /// Number of tasks currently in the queue.
    std::size_t size() const;

    /// True when no tasks are pending.
    bool empty() const;

    /// Remove all tasks.
    void clear();

private:
    std::deque<std::unique_ptr<Task>> queue_;
};

} // namespace liz