#pragma once

#include "engine/pluginloader/PluginCatalog.h"
#include "engine/pluginloader/PluginDescriptor.h"
#include "engine/pluginloader/PluginManifest.h"
#include "engine/pluginloader/PluginLoaderStatistics.h"

#include <memory>
#include <string>
#include <vector>

namespace liz {

// Forward declarations.
class EventBus;

/// Plugin Loader — simulated plugin loading system.
///
/// This Sprint does NOT load real DLLs or shared libraries.
/// Everything is simulated in-memory, prepared for future dynamic loading.
///
/// Publishes events to EventBus for all lifecycle transitions.
class PluginLoader {
public:
    PluginLoader();
    ~PluginLoader() = default;

    // Non-copyable.
    PluginLoader(const PluginLoader&) = delete;
    PluginLoader& operator=(const PluginLoader&) = delete;

    // ── Lifecycle ───────────────────────────────────────────────────────────

    /// Initialize the plugin loader.
    void initialize();

    /// Shut down the plugin loader and unload all plugins.
    void shutdown();

    /// Check if the loader is initialized.
    bool is_initialized() const { return initialized_; }

    // ── Event bus ──────────────────────────────────────────────────────────

    /// Set the EventBus for publishing plugin lifecycle events.
    void set_event_bus(EventBus* bus);

    // ── Registration ──────────────────────────────────────────────────────

    /// Register a plugin with its manifest. Creates a descriptor from the manifest.
    /// Returns the UUID of the registered plugin, or empty on failure.
    std::string register_plugin(const PluginManifest& manifest);

    // ── Loading ───────────────────────────────────────────────────────────

    /// Load a plugin by UUID. Simulated — no real DLL loading.
    /// Returns true if successfully loaded.
    bool load_plugin(const std::string& uuid);

    /// Unload a plugin by UUID.
    /// Returns true if successfully unloaded.
    bool unload_plugin(const std::string& uuid);

    /// Reload a plugin by UUID (unload then load).
    /// Returns true if successfully reloaded.
    bool reload_plugin(const std::string& uuid);

    /// Load all registered plugins.
    void load_all();

    /// Unload all loaded plugins.
    void unload_all();

    // ── Queries ────────────────────────────────────────────────────────────

    /// Check if a specific plugin is loaded.
    bool is_loaded(const std::string& uuid) const;

    /// Get all loaded plugin descriptors.
    std::vector<const PluginDescriptor*> loaded_plugins() const;

    /// Get all registered plugin names.
    std::vector<std::string> list_registered() const;

    /// Get the plugin catalog statistics.
    PluginLoaderStatistics statistics() const;

    // ── Cleanup ────────────────────────────────────────────────────────────

    /// Clear all plugins and reset statistics.
    void clear();

private:
    /// Internal event publishing using int code mapping.
    /// 0 = PluginLoading, 1 = PluginLoaded, 2 = PluginUnloading,
    /// 3 = PluginUnloaded, 4 = PluginReloaded, 5 = PluginLoadFailed
    void publish_event(int event_code, const std::string& message);

    bool initialized_ = false;
    EventBus* event_bus_ = nullptr;

    PluginCatalog catalog_;

    std::size_t stats_failed_    = 0;
    std::size_t stats_reloaded_  = 0;
    double     stats_total_load_time_ms_ = 0.0;
};

} // namespace liz
