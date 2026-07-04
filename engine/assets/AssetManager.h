#pragma once

#include "engine/assets/AssetHandle.h"
#include "engine/assets/AssetDatabase.h"
#include "engine/assets/AssetType.h"
#include "engine/events/EventType.h"

#include <cstddef>
#include <string>
#include <vector>

namespace liz {

class EventBus;

/// Statistics snapshot of the AssetManager.
struct AssetManagerStats {
    std::size_t total_created   = 0;
    std::size_t total_destroyed = 0;
    std::size_t total_loaded    = 0;
    std::size_t total_unloaded  = 0;
    std::size_t total_reloaded  = 0;
    std::size_t active_assets   = 0;
    std::size_t memory_bytes    = 0;
};

/// Central asset manager for the LIZ Engine.
///
/// Manages the lifecycle of all engine assets: creation, loading,
/// unloading, reloading, destruction.  Works alongside ResourceManager
/// — it does NOT replace it.
///
/// Publishes events to the EventBus when asset state changes.
/// Integrates with Logger for all operations.
class AssetManager {
public:
    AssetManager();
    ~AssetManager() = default;

    // Non-copyable.
    AssetManager(const AssetManager&) = delete;
    AssetManager& operator=(const AssetManager&) = delete;

    // ── Lifecycle ────────────────────────────────────────────────────────

    /// Initialize the asset manager.  Call once before other operations.
    bool initialize();

    /// Shut down the asset manager.  Clears all assets.
    void shutdown();

    // ── Creation / Destruction ───────────────────────────────────────────

    /// Create a new asset.  Returns a handle.
    AssetHandle create_asset(const std::string& name,
                             AssetType type,
                             const std::string& version = "1.0.0");

    /// Create a new asset with explicit size.
    AssetHandle create_asset(const std::string& name,
                             AssetType type,
                             std::size_t size_bytes,
                             const std::string& version = "1.0.0");

    /// Destroy an asset by UUID.  Returns true if found and removed.
    bool destroy_asset(const std::string& uuid);

    // ── Loading / Unloading ─────────────────────────────────────────────

    /// Simulate loading an asset (Unloaded -> Loading -> Loaded).
    bool load(const std::string& uuid);

    /// Simulate unloading an asset (Loaded -> Unloading -> Unloaded).
    bool unload(const std::string& uuid);

    /// Simulate reloading an asset (Loaded -> Unloaded -> Loading -> Loaded).
    bool reload(const std::string& uuid);

    // ── Query ─────────────────────────────────────────────────────────────

    /// Find an asset by UUID.  Returns nullptr if not found.
    Asset* find(const std::string& uuid) const;

    /// Find an asset by name.  Returns nullptr if not found.
    Asset* find_by_name(const std::string& name) const;

    /// Check if an asset exists by UUID.
    bool exists(const std::string& uuid) const;

    /// Get a handle by UUID.
    AssetHandle handle(const std::string& uuid) const;

    /// Get the names of all managed assets.
    std::vector<std::string> list() const;

    /// Number of managed assets.
    std::size_t count() const;

    // ── Statistics ──────────────────────────────────────────────────────

    /// Get a statistics snapshot.
    AssetManagerStats statistics() const;

    /// Log a summary of asset manager statistics.
    void log_statistics() const;

    // ── Bulk ─────────────────────────────────────────────────────────────

    /// Remove all assets.
    void clear();

    // ── EventBus ─────────────────────────────────────────────────────────

    /// Set the EventBus for publishing asset events.  Does NOT take ownership.
    void set_event_bus(EventBus* bus);

private:
    /// Publish an event if the EventBus is available.
    void publish_event(EventType type, const std::string& message);

    AssetDatabase   database_;
    EventBus*       event_bus_  = nullptr;
    bool            initialized_ = false;

    // Counters.
    std::size_t     total_created_   = 0;
    std::size_t     total_destroyed_ = 0;
    std::size_t     total_loaded_    = 0;
    std::size_t     total_unloaded_  = 0;
    std::size_t     total_reloaded_  = 0;
};

} // namespace liz
