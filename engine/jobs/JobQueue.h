#pragma once

#include "engine/jobs/Job.h"

#include <cstddef>
#include <memory>
#include <queue>

namespace liz {

/// FIFO queue for Jobs awaiting execution.
///
/// Jobs are stored as unique_ptr and processed in insertion order.
/// The priority field is stored in each Job but is NOT used for
/// scheduling in this sprint — the queue is strictly FIFO.
class JobQueue {
public:
    JobQueue() = default;

    // ── Operations ──────────────────────────────────────────────────────

    /// Add a job to the back of the queue.
    void push(std::unique_ptr<Job> job);

    /// Remove and return the job at the front of the queue.
    /// Returns nullptr if the queue is empty.
    std::unique_ptr<Job> pop();

    /// Access the job at the front without removing it.
    /// Returns nullptr if the queue is empty.
    Job* front() const;

    /// Check if the queue is empty.
    bool empty() const;

    /// Number of jobs in the queue.
    std::size_t size() const;

    /// Remove all jobs from the queue.
    void clear();

private:
    std::queue<std::unique_ptr<Job>> queue_;
};

} // namespace liz
