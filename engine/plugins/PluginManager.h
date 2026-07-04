#pragma once

#include "engine/plugins/PluginInterface.h"

#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace liz {

/// Registry that owns and manages plugin instances.
///
/// Plugins are stored by unique name.  Registering a plugin with a name
/// that already exists will fail (returns false).
///
/// No dynamic loading (.so/.dll) in this sprint — plugins are created
/// in code and handed to the manager via shared_ptr.
class PluginManager {
public:
    PluginManager() = default;

    // ── Registration ──────────────────────────────────────────────────────────
    /// Register a plugin.  Returns true on success, false if name is taken.
    bool register_plugin(std::shared_ptr<PluginInterface> plugin);

    /// Unregister a plugin by name.  Returns true if it existed and was removed.
    bool unregister(std::string_view name);

    /// Remove all plugins (calls on_unregister on each).
    void unregister_all();

    // ── Query ─────────────────────────────────────────────────────────────────
    /// Find a plugin by name.  Returns nullptr if not found.
    PluginInterface* find(std::string_view name) const;

    /// Total number of registered plugins.
    std::size_t count() const;

    /// True when no plugins are registered.
    bool empty() const;

    /// Get all plugin names.
    std::vector<std::string> list_names() const;

    /// Get all plugins in a given category.
    std::vector<std::string> list_by_category(std::string_view category) const;

private:
    /// Stored entry keeps the shared_ptr alive.
    struct Entry {
        std::shared_ptr<PluginInterface> plugin;
    };

    // Flat vector — fine for the small number of plugins we expect.
    // Future: replace with unordered_map if lookup becomes a bottleneck.
    std::vector<Entry> plugins_;
};

} // namespace liz