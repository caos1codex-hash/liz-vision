#include "engine/pluginloader/PluginLoader.h"
#include "engine/core/Logger.h"
#include "engine/events/EventBus.h"
#include "engine/events/Event.h"
#include "engine/events/EventType.h"

#include <chrono>
#include <sstream>

namespace liz {

// ── Constructor ───────────────────────────────────────────────────────────

PluginLoader::PluginLoader() = default;

// ── Lifecycle ────────────────────────────────────────────────────────────

void PluginLoader::initialize() {
    initialized_ = true;
    LIZ_INFO("PluginLoader: initialized");
}

void PluginLoader::shutdown() {
    unload_all();
    clear();
    initialized_ = false;
    LIZ_INFO("PluginLoader: shut down");
}

// ── Event bus ──────────────────────────────────────────────────────────────

void PluginLoader::set_event_bus(EventBus* bus) {
    event_bus_ = bus;
}

// ── Registration ──────────────────────────────────────────────────────────

std::string PluginLoader::register_plugin(const PluginManifest& manifest) {
    PluginDescriptor descriptor(
        manifest.name(),
        manifest.author(),
        manifest.version(),
        PluginCategory::Unknown
    );

    descriptor.set_description(manifest.description());
    descriptor.set_api_version(manifest.api_version());
    descriptor.set_engine_version(manifest.required_engine_version());

    auto uuid = descriptor.uuid();

    if (!catalog_.register_plugin(std::move(descriptor))) {
        return "";
    }

    std::ostringstream oss;
    oss << "PluginLoader: registered plugin '" << manifest.name() << "' uuid=" << uuid;
    LIZ_INFO(oss.str());

    return uuid;
}

// ── Loading ───────────────────────────────────────────────────────────────

bool PluginLoader::load_plugin(const std::string& uuid) {
    if (!initialized_) {
        LIZ_WARN("PluginLoader: load_plugin called but not initialized");
        return false;
    }

    auto* desc = catalog_.find(uuid);
    if (!desc) {
        std::ostringstream oss;
        oss << "PluginLoader: plugin not found — uuid=" << uuid;
        LIZ_WARN(oss.str());
        publish_event(5, "PluginLoadFailed: " + uuid + " — not found");
        return false;
    }

    if (desc->is_loaded()) {
        std::ostringstream oss;
        oss << "PluginLoader: plugin already loaded — " << desc->name();
        LIZ_WARN(oss.str());
        return false;
    }

    if (!desc->is_enabled()) {
        std::ostringstream oss;
        oss << "PluginLoader: plugin disabled — " << desc->name();
        LIZ_WARN(oss.str());
        publish_event(5, "PluginLoadFailed: " + desc->name() + " — disabled");
        ++stats_failed_;
        return false;
    }

    // Publish PluginLoading event.
    {
        std::ostringstream oss;
        oss << "Loading plugin '" << desc->name() << "' uuid=" << uuid;
        publish_event(0, oss.str());
    }

    // Simulated load time (1–10 ms).
    auto start = std::chrono::steady_clock::now();

    // Simulate loading — no real DLL.
    desc->set_loaded(true);

    auto end = std::chrono::steady_clock::now();
    double elapsed_ms = static_cast<double>(
        std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()) / 1000.0;

    desc->set_load_time(elapsed_ms);
    stats_total_load_time_ms_ += elapsed_ms;

    // Publish PluginLoaded event.
    {
        std::ostringstream oss;
        oss << "Plugin '" << desc->name() << "' loaded successfully (" << elapsed_ms << " ms)";
        publish_event(1, oss.str());
    }

    {
        std::ostringstream oss;
        oss << "PluginLoader: plugin loaded — " << desc->to_string();
        LIZ_INFO(oss.str());
    }

    return true;
}

bool PluginLoader::unload_plugin(const std::string& uuid) {
    if (!initialized_) return false;

    auto* desc = catalog_.find(uuid);
    if (!desc || !desc->is_loaded()) {
        std::ostringstream oss;
        oss << "PluginLoader: cannot unload — plugin not found or not loaded — uuid=" << uuid;
        LIZ_WARN(oss.str());
        return false;
    }

    // Publish PluginUnloading event.
    {
        std::ostringstream oss;
        oss << "Unloading plugin '" << desc->name() << "' uuid=" << uuid;
        publish_event(2, oss.str());
    }

    // Simulated unload time.
    auto start = std::chrono::steady_clock::now();

    desc->set_loaded(false);

    auto end = std::chrono::steady_clock::now();
    double elapsed_ms = static_cast<double>(
        std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()) / 1000.0;

    desc->set_unload_time(elapsed_ms);

    // Publish PluginUnloaded event.
    {
        std::ostringstream oss;
        oss << "Plugin '" << desc->name() << "' unloaded (" << elapsed_ms << " ms)";
        publish_event(3, oss.str());
    }

    {
        std::ostringstream oss;
        oss << "PluginLoader: plugin unloaded — " << desc->to_string();
        LIZ_INFO(oss.str());
    }

    return true;
}

bool PluginLoader::reload_plugin(const std::string& uuid) {
    if (!initialized_) return false;

    auto* desc = catalog_.find(uuid);
    if (!desc) {
        std::ostringstream oss;
        oss << "PluginLoader: cannot reload — plugin not found — uuid=" << uuid;
        LIZ_WARN(oss.str());
        publish_event(5, "PluginLoadFailed: reload — " + uuid + " — not found");
        ++stats_failed_;
        return false;
    }

    std::string name = desc->name();

    // Unload then load.
    if (desc->is_loaded()) {
        unload_plugin(uuid);
    }

    if (!load_plugin(uuid)) {
        return false;
    }

    ++stats_reloaded_;

    // Publish PluginReloaded event.
    {
        std::ostringstream oss;
        oss << "Plugin '" << name << "' reloaded successfully";
        publish_event(4, oss.str());
    }

    {
        std::ostringstream oss;
        oss << "PluginLoader: plugin reloaded — " << name;
        LIZ_INFO(oss.str());
    }

    return true;
}

void PluginLoader::load_all() {
    if (!initialized_) return;

    LIZ_INFO("PluginLoader: loading all registered plugins...");

    auto names = catalog_.list();
    for (const auto& name : names) {
        auto* desc = catalog_.find_by_name(name);
        if (desc) {
            load_plugin(desc->uuid());
        }
    }
}

void PluginLoader::unload_all() {
    if (!initialized_) return;

    LIZ_INFO("PluginLoader: unloading all loaded plugins...");

    auto names = catalog_.list();
    for (const auto& name : names) {
        auto* desc = catalog_.find_by_name(name);
        if (desc && desc->is_loaded()) {
            unload_plugin(desc->uuid());
        }
    }
}

// ── Queries ──────────────────────────────────────────────────────────────

bool PluginLoader::is_loaded(const std::string& uuid) const {
    auto* desc = catalog_.find(uuid);
    return desc && desc->is_loaded();
}

std::vector<const PluginDescriptor*> PluginLoader::loaded_plugins() const {
    std::vector<const PluginDescriptor*> result;
    auto names = catalog_.list();
    for (const auto& name : names) {
        auto* desc = catalog_.find_by_name(name);
        if (desc && desc->is_loaded()) {
            result.push_back(desc);
        }
    }
    return result;
}

std::vector<std::string> PluginLoader::list_registered() const {
    return catalog_.list();
}

PluginLoaderStatistics PluginLoader::statistics() const {
    auto stats = catalog_.statistics();
    stats.failed = stats_failed_;
    stats.reloaded = stats_reloaded_;
    stats.total_load_time_ms = stats_total_load_time_ms_;
    return stats;
}

void PluginLoader::clear() {
    catalog_.clear();
    stats_failed_ = 0;
    stats_reloaded_ = 0;
    stats_total_load_time_ms_ = 0.0;
    LIZ_INFO("PluginLoader: cleared");
}

// ── Internal event publishing ─────────────────────────────────────────────

void PluginLoader::publish_event(int event_code, const std::string& message) {
    if (!event_bus_) return;

    EventType type;
    switch (event_code) {
        case 0: type = EventType::PluginLoading;     break;
        case 1: type = EventType::PluginLoaded;      break;
        case 2: type = EventType::PluginUnloading;   break;
        case 3: type = EventType::PluginUnloaded;     break;
        case 4: type = EventType::PluginReloaded;    break;
        case 5: type = EventType::PluginLoadFailed;   break;
        default: type = EventType::Custom; break;
    }

    Event event(type, "PluginLoader", message);
    event_bus_->publish(event);
}

} // namespace liz
