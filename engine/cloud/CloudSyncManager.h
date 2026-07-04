#pragma once

#include "engine/cloud/CloudSyncItem.h"
#include "engine/cloud/CloudSyncQueue.h"
#include "engine/cloud/CloudTypes.h"

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

namespace liz {

// Forward declarations.
class EventBus;

/// Cloud Sync Manager — offline-first simulated cloud synchronization.
///
/// Manages a FIFO queue of sync items and processes them with simulated
/// cloud sync behavior:
///   - 70% chance: item transitions to Synced
///   - 30% chance: item transitions to Conflict
///
/// Publishes events to EventBus for each state transition.
class CloudSyncManager {
public:
    CloudSyncManager();
    ~CloudSyncManager() = default;

    // Non-copyable.
    CloudSyncManager(const CloudSyncManager&) = delete;
    CloudSyncManager& operator=(const CloudSyncManager&) = delete;

    // ── Lifecycle ───────────────────────────────────────────────────────────

    /// Initialize the manager.
    void initialize();

    /// Shut down the manager and clear all items.
    void shutdown();

    /// Check if the manager is initialized.
    bool is_initialized() const { return initialized_; }

    // ── Event bus ──────────────────────────────────────────────────────────

    /// Set the EventBus for publishing sync events.
    void set_event_bus(EventBus* bus);

    // ── Queue operations ──────────────────────────────────────────────────

    /// Enqueue an item for cloud sync.
    void enqueue(CloudSyncItem item);

    /// Process all items in the queue (FIFO order).
    void process_all();

    /// Process a single item. Returns true if processed.
    bool process_item(CloudSyncItem& item);

    // ── Statistics ─────────────────────────────────────────────────────────

    /// Number of items currently pending in the queue.
    std::size_t pending_count() const;

    /// Total items that have been synced.
    std::size_t synced_count() const { return stats_synced_; }

    /// Total items that have conflicts.
    std::size_t conflict_count() const { return stats_conflicts_; }

    /// Total items that have failed.
    std::size_t failed_count() const { return stats_failed_; }

    /// Total sync time accumulated (ms, simulated).
    double total_sync_time_ms() const { return stats_sync_time_ms_; }

    /// Get a copy of all processed (synced/conflict/failed) items.
    const std::vector<CloudSyncItem>& processed_items() const { return processed_items_; }

    /// List all pending item names.
    std::vector<std::string> list_pending() const;

    /// Clear all items and reset statistics.
    void clear();

private:
    /// Internal event publishing (avoids direct EventType include in header).
    /// Uses event_code int mapping:
    ///   0 = CloudSyncStarted
    ///   1 = CloudSyncCompleted
    ///   2 = CloudSyncFailed
    ///   3 = CloudConflictDetected
    void publish_event(int event_code, const std::string& message);

    bool initialized_ = false;
    EventBus* event_bus_ = nullptr;

    CloudSyncQueue queue_;
    std::vector<CloudSyncItem> processed_items_;

    std::size_t stats_synced_    = 0;
    std::size_t stats_conflicts_ = 0;
    std::size_t stats_failed_    = 0;
    double     stats_sync_time_ms_ = 0.0;
};

} // namespace liz
