#pragma once

#include "engine/pluginloader/PluginDescriptor.h"
#include "engine/pluginloader/PluginLoaderStatistics.h"

#include <cstddef>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace liz {

/// In-memory catalog of registered plugin descriptors.
///
/// Provides register/unregister/find/exists/list operations.
/// No JSON, no filesystem — purely in-memory.
class PluginCatalog {
public:
    PluginCatalog() = default;
    ~PluginCatalog() = default;

    // Non-copyable.
    PluginCatalog(const PluginCatalog&) = delete;
    PluginCatalog& operator=(const PluginCatalog&) = delete;

    // ── Operations ────────────────────────────────────────────────────────

    /// Register a plugin descriptor. Returns false if already exists.
    bool register_plugin(PluginDescriptor descriptor);

    /// Unregister a plugin by UUID. Returns false if not found.
    bool unregister_plugin(const std::string& uuid);

    /// Find a plugin descriptor by UUID. Returns nullptr if not found.
    PluginDescriptor* find(const std::string& uuid);

    /// Find a plugin descriptor by UUID (const).
    const PluginDescriptor* find(const std::string& uuid) const;

    /// Find a plugin descriptor by name. Returns nullptr if not found.
    PluginDescriptor* find_by_name(const std::string& name);

    /// Find a plugin descriptor by name (const).
    const PluginDescriptor* find_by_name(const std::string& name) const;

    /// Check if a plugin with the given UUID exists.
    bool exists(const std::string& uuid) const;

    /// List all plugin names.
    std::vector<std::string> list() const;

    /// Number of registered plugins.
    std::size_t count() const;

    /// Get statistics for all plugins in the catalog.
    PluginLoaderStatistics statistics() const;

    /// Remove all plugins and reset statistics.
    void clear();

private:
    std::unordered_map<std::string, PluginDescriptor> plugins_;
};

} // namespace liz
