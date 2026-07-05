#include "engine/config/ConfigurationManager.h"
#include "engine/events/EventBus.h"
#include "engine/events/Event.h"
#include "engine/events/EventType.h"
#include "engine/core/Logger.h"

#include <sstream>

namespace liz {

ConfigurationManager::ConfigurationManager() = default;

ConfigurationManager::~ConfigurationManager() {
    if (!configs_.empty()) {
        shutdown();
    }
}

// ── Lifecycle ────────────────────────────────────────────────────────────

void ConfigurationManager::initialize() {
    LIZ_INFO("ConfigurationManager: initialized");
}

void ConfigurationManager::shutdown() {
    clear();
    event_bus_ = nullptr;
    LIZ_INFO("ConfigurationManager: shutdown");
}

// ── EventBus ────────────────────────────────────────────────────────────

void ConfigurationManager::set_event_bus(EventBus* bus) {
    event_bus_ = bus;
    LIZ_INFO("ConfigurationManager: connected to EventBus");
}

// ── Configuration management ─────────────────────────────────────────────

Configuration* ConfigurationManager::create_configuration(const std::string& name,
                                                             const std::string& description) {
    auto config = std::make_unique<Configuration>(name, description);
    const std::string& uuid = config->uuid();

    {
        std::ostringstream oss;
        oss << "ConfigurationManager: created \"" << name << "\" uuid=" << uuid;
        LIZ_INFO(oss.str());
    }

    publish_event(0, "Configuration created: " + name + " (" + uuid + ")");

    auto* ptr = config.get();
    configs_[uuid] = std::move(config);

    // Auto-activate first config.
    if (active_uuid_.empty()) {
        active_uuid_ = uuid;
        publish_event(2, "Configuration activated: " + name + " (" + uuid + ")");
    }

    recompute_stats();
    return ptr;
}

bool ConfigurationManager::destroy_configuration(const std::string& uuid) {
    auto it = configs_.find(uuid);
    if (it == configs_.end()) return false;

    std::string name = it->second->name();
    configs_.erase(it);

    if (active_uuid_ == uuid) {
        active_uuid_.clear();
    }

    {
        std::ostringstream oss;
        oss << "ConfigurationManager: destroyed \"" << name << "\" uuid=" << uuid;
        LIZ_INFO(oss.str());
    }

    publish_event(1, "Configuration destroyed: " + name + " (" + uuid + ")");
    recompute_stats();
    return true;
}

bool ConfigurationManager::set_active(const std::string& uuid) {
    if (!configs_.count(uuid)) return false;

    active_uuid_ = uuid;
    auto* config = configs_[uuid].get();

    {
        std::ostringstream oss;
        oss << "ConfigurationManager: activated \"" << config->name() << "\"";
        LIZ_INFO(oss.str());
    }

    publish_event(2, "Configuration activated: " + config->name() + " (" + uuid + ")");
    return true;
}

Configuration* ConfigurationManager::active() {
    if (active_uuid_.empty()) return nullptr;
    auto it = configs_.find(active_uuid_);
    if (it == configs_.end()) return nullptr;
    return it->second.get();
}

const Configuration* ConfigurationManager::active() const {
    if (active_uuid_.empty()) return nullptr;
    auto it = configs_.find(active_uuid_);
    if (it == configs_.end()) return nullptr;
    return it->second.get();
}

Configuration* ConfigurationManager::find(const std::string& uuid) {
    auto it = configs_.find(uuid);
    if (it == configs_.end()) return nullptr;
    return it->second.get();
}

const Configuration* ConfigurationManager::find(const std::string& uuid) const {
    auto it = configs_.find(uuid);
    if (it == configs_.end()) return nullptr;
    return it->second.get();
}

std::vector<std::string> ConfigurationManager::list() const {
    std::vector<std::string> uuids;
    uuids.reserve(configs_.size());
    for (const auto& [uuid, config] : configs_) {
        uuids.push_back(uuid);
    }
    return uuids;
}

void ConfigurationManager::clear() {
    configs_.clear();
    active_uuid_.clear();
    stats_ = ConfigurationStatistics{};
    LIZ_INFO("ConfigurationManager: cleared");
}

ConfigurationStatistics ConfigurationManager::statistics() const {
    return stats_;
}

// ── Event publishing ────────────────────────────────────────────────────

void ConfigurationManager::publish_event(int event_code, const std::string& message) {
    if (!event_bus_) return;

    EventType type;
    switch (event_code) {
        case 0: type = EventType::ConfigurationCreated;      break;
        case 1: type = EventType::ConfigurationDestroyed;    break;
        case 2: type = EventType::ConfigurationActivated;    break;
        case 3: type = EventType::ConfigurationValueChanged; break;
        case 4: type = EventType::ConfigurationReset;         break;
        default: type = EventType::Custom;                     break;
    }

    Event event(type, "ConfigurationManager", message);
    event_bus_->publish(event);
}

// ── Internal ────────────────────────────────────────────────────────────

void ConfigurationManager::recompute_stats() {
    stats_.created = configs_.size();
    stats_.destroyed = 0; // cumulative; not tracked per-destroy in this sprint
    stats_.active = active_uuid_.empty() ? 0 : 1;

    stats_.sections = 0;
    stats_.values = 0;
    stats_.modified_values = 0;

    for (const auto& [uuid, config] : configs_) {
        stats_.sections += config->section_count();
        stats_.values += config->total_values();
        stats_.modified_values += config->total_modified();
    }
}

} // namespace liz
