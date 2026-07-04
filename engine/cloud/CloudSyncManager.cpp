#include "engine/cloud/CloudSyncManager.h"
#include "engine/core/Logger.h"
#include "engine/events/EventBus.h"
#include "engine/events/Event.h"
#include "engine/events/EventType.h"

#include <chrono>
#include <random>
#include <sstream>

namespace liz {

// ── Constructor ───────────────────────────────────────────────────────────

CloudSyncManager::CloudSyncManager() = default;

// ── Lifecycle ────────────────────────────────────────────────────────────

void CloudSyncManager::initialize() {
    initialized_ = true;
    LIZ_INFO("CloudSyncManager: initialized");
}

void CloudSyncManager::shutdown() {
    clear();
    initialized_ = false;
    LIZ_INFO("CloudSyncManager: shut down");
}

// ── Event bus ──────────────────────────────────────────────────────────────

void CloudSyncManager::set_event_bus(EventBus* bus) {
    event_bus_ = bus;
}

// ── Queue operations ──────────────────────────────────────────────────────

void CloudSyncManager::enqueue(CloudSyncItem item) {
    {
        std::ostringstream oss;
        oss << "CloudSyncManager: enqueued item — " << item.to_string();
        LIZ_INFO(oss.str());
    }

    auto ptr = std::make_unique<CloudSyncItem>(std::move(item));
    queue_.enqueue(std::move(ptr));
}

void CloudSyncManager::process_all() {
    if (!initialized_) {
        LIZ_WARN("CloudSyncManager: process_all called but not initialized");
        return;
    }

    LIZ_INFO("CloudSyncManager: processing all items...");

    // Process until the queue is empty.
    // Each call to dequeue returns the front item; we process it in-place.
    while (!queue_.empty()) {
        auto item = queue_.dequeue();
        if (item) {
            process_item(*item);
            processed_items_.push_back(std::move(*item));
        }
    }
}

bool CloudSyncManager::process_item(CloudSyncItem& item) {
    if (!initialized_) {
        return false;
    }

    // Transition: Pending → Syncing
    item.set_state(CloudSyncState::Syncing);
    publish_event(0, item.to_string() + " — syncing started");

    {
        std::ostringstream oss;
        oss << "CloudSyncManager: syncing item — " << item.to_string();
        LIZ_INFO(oss.str());
    }

    // Simulated sync time (random 5–50 ms).
    auto start = std::chrono::steady_clock::now();

    // Simulated cloud sync result: 70% Synced, 30% Conflict
    auto now = std::chrono::steady_clock::now().time_since_epoch().count();
    std::mt19937_64 rng(static_cast<std::mt19937_64::result_type>(now));
    std::uniform_int_distribution<int> dist(1, 100);
    int roll = dist(rng);

    auto end = std::chrono::steady_clock::now();
    double elapsed_ms = static_cast<double>(
        std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()) / 1000.0;
    stats_sync_time_ms_ += elapsed_ms;

    if (roll <= 70) {
        // 70% Synced
        item.set_state(CloudSyncState::Synced);
        item.set_synced_at(static_cast<std::uint64_t>(end.time_since_epoch().count() / 1000000));
        ++stats_synced_;

        publish_event(1, item.to_string() + " — sync completed");

        {
            std::ostringstream oss;
            oss << "CloudSyncManager: synced item — " << item.to_string();
            LIZ_INFO(oss.str());
        }
    } else {
        // 30% Conflict
        item.set_state(CloudSyncState::Conflict);
        item.set_error_message("Version conflict detected — remote data differs from local");
        ++stats_conflicts_;

        publish_event(3, item.to_string() + " — conflict detected");

        {
            std::ostringstream oss;
            oss << "CloudSyncManager: conflict detected — " << item.to_string();
            LIZ_INFO(oss.str());
        }
    }

    return true;
}

// ── Statistics ────────────────────────────────────────────────────────────

std::size_t CloudSyncManager::pending_count() const {
    return queue_.size();
}

std::vector<std::string> CloudSyncManager::list_pending() const {
    std::vector<std::string> names;
    // CloudSyncQueue does not expose iteration, so we return size-based info.
    names.push_back(std::to_string(queue_.size()) + " pending items");
    return names;
}

void CloudSyncManager::clear() {
    queue_.clear();
    processed_items_.clear();
    stats_synced_ = 0;
    stats_conflicts_ = 0;
    stats_failed_ = 0;
    stats_sync_time_ms_ = 0.0;
    LIZ_INFO("CloudSyncManager: cleared");
}

// ── Internal event publishing ─────────────────────────────────────────────

void CloudSyncManager::publish_event(int event_code, const std::string& message) {
    if (!event_bus_) return;

    EventType type;
    switch (event_code) {
        case 0: type = EventType::CloudSyncStarted;      break;
        case 1: type = EventType::CloudSyncCompleted;     break;
        case 2: type = EventType::CloudSyncFailed;        break;
        case 3: type = EventType::CloudConflictDetected;  break;
        default: type = EventType::Custom; break;
    }

    Event event(type, "CloudSyncManager", message);
    event_bus_->publish(event);
}

} // namespace liz
