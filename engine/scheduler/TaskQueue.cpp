#include "engine/scheduler/TaskQueue.h"

namespace liz {

void TaskQueue::push(std::unique_ptr<Task> task) {
    queue_.push_back(std::move(task));
}

std::unique_ptr<Task> TaskQueue::pop() {
    if (queue_.empty()) {
        return nullptr;
    }
    auto task = std::move(queue_.front());
    queue_.pop_front();
    return task;
}

Task* TaskQueue::front() {
    return queue_.empty() ? nullptr : queue_.front().get();
}

std::size_t TaskQueue::size() const {
    return queue_.size();
}

bool TaskQueue::empty() const {
    return queue_.empty();
}

void TaskQueue::clear() {
    queue_.clear();
}

} // namespace liz