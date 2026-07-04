#pragma once

#include "engine/cloud/CloudSyncItem.h"

#include <cstddef>
#include <memory>
#include <queue>

namespace liz {

/// FIFO queue for cloud sync items.
///
/// Provides simple enqueue/dequeue operations with size tracking.
/// The queue owns its items via unique_ptr.
class CloudSyncQueue {
public:
    CloudSyncQueue() = default;
    ~CloudSyncQueue() = default;

    // Non-copyable.
    CloudSyncQueue(const CloudSyncQueue&) = delete;
    CloudSyncQueue& operator=(const CloudSyncQueue&) = delete;

    // ── Queue operations ───────────────────────────────────────────────────

    /// Add an item to the back of the queue.
    void enqueue(std::unique_ptr<CloudSyncItem> item);

    /// Remove and return the item at the front of the queue.
    /// Returns nullptr if the queue is empty.
    std::unique_ptr<CloudSyncItem> dequeue();

    /// Peek at the front item without removing it.
    /// Returns nullptr if the queue is empty.
    const CloudSyncItem* front() const;

    /// Number of items in the queue.
    std::size_t size() const { return static_cast<std::size_t>(queue_.size()); }

    /// Check if the queue is empty.
    bool empty() const { return queue_.empty(); }

    /// Remove all items from the queue.
    void clear();

    /// Generate a formatted summary string.
    std::string to_string() const;

private:
    std::queue<std::unique_ptr<CloudSyncItem>> queue_;
};

} // namespace liz
