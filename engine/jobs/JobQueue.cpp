#include "engine/jobs/JobQueue.h"

namespace liz {

// ── Operations ────────────────────────────────────────────────────────────

void JobQueue::push(std::unique_ptr<Job> job) {
    queue_.push(std::move(job));
}

std::unique_ptr<Job> JobQueue::pop() {
    if (queue_.empty()) return nullptr;

    auto job = std::move(queue_.front());
    queue_.pop();
    return job;
}

Job* JobQueue::front() const {
    if (queue_.empty()) return nullptr;
    return queue_.front().get();
}

bool JobQueue::empty() const {
    return queue_.empty();
}

std::size_t JobQueue::size() const {
    return queue_.size();
}

void JobQueue::clear() {
    while (!queue_.empty()) {
        queue_.pop();
    }
}

} // namespace liz
