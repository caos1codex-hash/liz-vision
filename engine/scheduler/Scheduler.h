#pragma once

#include "engine/scheduler/TaskQueue.h"

#include <cstddef>

namespace liz {

/// Basic FIFO task scheduler.
///
/// Consumes tasks from a TaskQueue one at a time, in order.
/// No multithreading in this sprint — single-threaded, synchronous execution.
class Scheduler {
public:
    Scheduler() = default;

    // ── Task submission ──────────────────────────────────────────────────────
    /// Submit a task for execution (takes ownership).
    /// Returns the assigned TaskId.
    TaskId submit(std::unique_ptr<Task> task);

    /// Convenience: create and submit a named task inline.
    /// Returns the assigned TaskId.
    TaskId submit(std::string name, Task::ExecuteFn fn);

    // ── Execution ────────────────────────────────────────────────────────────
    /// Execute the next pending task (FIFO order).
    /// Returns true if a task was executed, false if queue was empty.
    bool run_next();

    /// Execute all pending tasks until the queue is drained.
    /// Returns the number of tasks executed.
    std::size_t run_all();

    // ── Query ────────────────────────────────────────────────────────────────
    std::size_t pending_count() const;
    bool        has_pending() const;

private:
    TaskQueue queue_;
};

} // namespace liz