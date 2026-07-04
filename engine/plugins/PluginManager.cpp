#include "engine/plugins/PluginManager.h"
#include "engine/core/Logger.h"

#include <algorithm>
#include <sstream>

namespace liz {

// ── Registration ──────────────────────────────────────────────────────────────
bool PluginManager::register_plugin(std::shared_ptr<PluginInterface> plugin) {
    if (!plugin) {
        LIZ_WARN("PluginManager: attempted to register null plugin");
        return false;
    }

    auto name = std::string(plugin->name());

    // Check for duplicate name.
    for (const auto& entry : plugins_) {
        if (entry.plugin->name() == name) {
            std::ostringstream oss;
            oss << "PluginManager: plugin '" << name << "' already registered";
            LIZ_WARN(oss.str());
            return false;
        }
    }

    plugin->on_register();

    std::ostringstream oss;
    oss << "PluginManager: registered plugin '"
        << name << "' v" << plugin->version()
        << " [" << plugin->category() << "]";
    LIZ_INFO(oss.str());

    plugins_.push_back(Entry{std::move(plugin)});
    return true;
}

bool PluginManager::unregister(std::string_view name) {
    auto it = std::find_if(plugins_.begin(), plugins_.end(),
        [&name](const Entry& e) { return e.plugin->name() == name; });

    if (it == plugins_.end()) {
        std::ostringstream oss;
        oss << "PluginManager: plugin '" << name << "' not found";
        LIZ_WARN(oss.str());
        return false;
    }

    it->plugin->on_unregister();

    std::ostringstream oss;
    oss << "PluginManager: unregistered plugin '" << name << "'";
    LIZ_INFO(oss.str());

    plugins_.erase(it);
    return true;
}

void PluginManager::unregister_all() {
    for (auto& entry : plugins_) {
        entry.plugin->on_unregister();
    }

    auto count = plugins_.size();
    plugins_.clear();

    std::ostringstream oss;
    oss << "PluginManager: unregistered all plugins (" << count << " total)";
    LIZ_INFO(oss.str());
}

// ── Query ─────────────────────────────────────────────────────────────────────
PluginInterface* PluginManager::find(std::string_view name) const {
    for (const auto& entry : plugins_) {
        if (entry.plugin->name() == name) {
            return entry.plugin.get();
        }
    }
    return nullptr;
}

std::size_t PluginManager::count() const {
    return plugins_.size();
}

bool PluginManager::empty() const {
    return plugins_.empty();
}

std::vector<std::string> PluginManager::list_names() const {
    std::vector<std::string> names;
    names.reserve(plugins_.size());
    for (const auto& entry : plugins_) {
        names.emplace_back(entry.plugin->name());
    }
    return names;
}

std::vector<std::string> PluginManager::list_by_category(std::string_view category) const {
    std::vector<std::string> names;
    for (const auto& entry : plugins_) {
        if (entry.plugin->category() == category) {
            names.emplace_back(entry.plugin->name());
        }
    }
    return names;
}

} // namespace liz