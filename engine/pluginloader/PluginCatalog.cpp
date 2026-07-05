#include "engine/pluginloader/PluginCatalog.h"
#include "engine/core/Logger.h"

#include <sstream>

namespace liz {

// ── Operations ───────────────────────────────────────────────────────────────

bool PluginCatalog::register_plugin(PluginDescriptor descriptor) {
    auto uuid = descriptor.uuid();
    if (plugins_.find(uuid) != plugins_.end()) {
        std::ostringstream oss;
        oss << "PluginCatalog: plugin already registered — uuid=" << uuid;
        LIZ_WARN(oss.str());
        return false;
    }

    plugins_.emplace(uuid, std::move(descriptor));

    std::ostringstream oss;
    oss << "PluginCatalog: registered plugin — uuid=" << uuid;
    LIZ_INFO(oss.str());

    return true;
}

bool PluginCatalog::unregister_plugin(const std::string& uuid) {
    auto it = plugins_.find(uuid);
    if (it == plugins_.end()) {
        std::ostringstream oss;
        oss << "PluginCatalog: plugin not found — uuid=" << uuid;
        LIZ_WARN(oss.str());
        return false;
    }

    plugins_.erase(it);

    std::ostringstream oss;
    oss << "PluginCatalog: unregistered plugin — uuid=" << uuid;
    LIZ_INFO(oss.str());

    return true;
}

PluginDescriptor* PluginCatalog::find(const std::string& uuid) {
    auto it = plugins_.find(uuid);
    return (it != plugins_.end()) ? &(it->second) : nullptr;
}

const PluginDescriptor* PluginCatalog::find(const std::string& uuid) const {
    auto it = plugins_.find(uuid);
    return (it != plugins_.end()) ? &(it->second) : nullptr;
}

PluginDescriptor* PluginCatalog::find_by_name(const std::string& name) {
    for (auto& pair : plugins_) {
        if (pair.second.name() == name) {
            return &(pair.second);
        }
    }
    return nullptr;
}

const PluginDescriptor* PluginCatalog::find_by_name(const std::string& name) const {
    for (const auto& pair : plugins_) {
        if (pair.second.name() == name) {
            return &(pair.second);
        }
    }
    return nullptr;
}

bool PluginCatalog::exists(const std::string& uuid) const {
    return plugins_.find(uuid) != plugins_.end();
}

std::vector<std::string> PluginCatalog::list() const {
    std::vector<std::string> names;
    names.reserve(plugins_.size());
    for (const auto& pair : plugins_) {
        names.push_back(pair.second.name());
    }
    return names;
}

std::size_t PluginCatalog::count() const {
    return plugins_.size();
}

PluginLoaderStatistics PluginCatalog::statistics() const {
    PluginLoaderStatistics stats;
    stats.registered = plugins_.size();

    for (const auto& pair : plugins_) {
        if (pair.second.is_loaded()) {
            ++stats.loaded;
        }
        if (!pair.second.is_enabled()) {
            ++stats.disabled;
        }
    }

    return stats;
}

void PluginCatalog::clear() {
    plugins_.clear();
    LIZ_INFO("PluginCatalog: cleared");
}

} // namespace liz
