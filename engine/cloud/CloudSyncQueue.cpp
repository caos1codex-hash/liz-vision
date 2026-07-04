#include "engine/cloud/CloudSyncQueue.h"

#include <sstream>

namespace liz {

// ── Queue operations ───────────────────────────────────────────────────────

void CloudSyncQueue::enqueue(std::unique_ptr<CloudSyncItem> item) {
    queue_.push(std::move(item));
}

std::unique_ptr<CloudSyncItem> CloudSyncQueue::dequeue() {
    if (queue_.empty()) {
        return nullptr;
    }
    auto item = std::move(queue_.front());
    queue_.pop();
    return item;
}

const CloudSyncItem* CloudSyncQueue::front() const {
    if (queue_.empty()) {
        return nullptr;
    }
    return queue_.front().get();
}

void CloudSyncQueue::clear() {
    while (!queue_.empty()) {
        queue_.pop();
    }
}

// ── to_string ────────────────────────────────────────────────────────────

std::string CloudSyncQueue::to_string() const {
    std::ostringstream oss;
    oss << "CloudSyncQueue[size=" << size() << "]";
    return oss.str();
}

} // namespace liz
