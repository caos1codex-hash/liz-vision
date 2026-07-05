#pragma once

#include "engine/config/Configuration.h"
#include "engine/config/ConfigurationStatistics.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace liz {

// Forward declarations.
class EventBus;
class Event;

/// Central manager for Configuration profiles.
///
/// Allows creating, destroying, and activating configurations.
/// Publishes events via EventBus when state changes.
class ConfigurationManager {
public:
    ConfigurationManager();
    ~ConfigurationManager();

    // Non-copyable.
    ConfigurationManager(const ConfigurationManager&) = delete;
    ConfigurationManager& operator=(const ConfigurationManager&) = delete;

    // ── Lifecycle ──────────────────────────────────────────────────────

    void initialize();
    void shutdown();

    // ── EventBus ───────────────────────────────────────────────────────

    void set_event_bus(EventBus* bus);

    // ── Configuration management ───────────────────────────────────────

    /// Create a new configuration.  Returns pointer (owned by manager).
    Configuration* create_configuration(const std::string& name, const std::string& description = "");

    /// Destroy a configuration by UUID.  Returns false if not found.
    bool destroy_configuration(const std::string& uuid);

    /// Set the active configuration by UUID.  Returns false if not found.
    bool set_active(const std::string& uuid);

    /// Get the active configuration.  Returns nullptr if none.
    Configuration* active();
    const Configuration* active() const;

    /// Find a configuration by UUID.
    Configuration* find(const std::string& uuid);
    const Configuration* find(const std::string& uuid) const;

    /// List all configuration UUIDs.
    std::vector<std::string> list() const;

    /// Clear all configurations.
    void clear();

    /// Get cumulative statistics.
    ConfigurationStatistics statistics() const;

private:
    /// Publish an event via the EventBus.
    void publish_event(int event_code, const std::string& message);

    /// Recompute statistics from current state.
    void recompute_stats();

    EventBus* event_bus_ = nullptr;

    std::unordered_map<std::string, std::unique_ptr<Configuration>> configs_;
    std::string active_uuid_;

    ConfigurationStatistics stats_;
};

} // namespace liz
