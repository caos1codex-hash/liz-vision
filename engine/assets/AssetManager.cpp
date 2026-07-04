#include "engine/assets/AssetManager.h"
#include "engine/events/EventBus.h"
#include "engine/events/Event.h"
#include "engine/core/Logger.h"

#include <algorithm>
#include <sstream>

namespace liz {

// ── Constructor ───────────────────────────────────────────────────────────

AssetManager::AssetManager() = default;

// ── Lifecycle ─────────────────────────────────────────────────────────────

bool AssetManager::initialize() {
    if (initialized_) {
        LIZ_WARN("AssetManager: already initialized");
        return true;
    }

    LIZ_INFO("AssetManager: initializing...");
    initialized_ = true;
    publish_event(EventType::RuntimeInitialized,
                  "AssetManager initialized");
    return true;
}

void AssetManager::shutdown() {
    if (!initialized_) {
        return;
    }

    LIZ_INFO("AssetManager: shutting down...");
    clear();
    initialized_ = false;
}

// ── Creation / Destruction ─────────────────────────────────────────────────

AssetHandle AssetManager::create_asset(const std::string& name,
                                        AssetType type,
                                        const std::string& version) {
    return create_asset(name, type, 0, version);
}

AssetHandle AssetManager::create_asset(const std::string& name,
                                        AssetType type,
                                        std::size_t size_bytes,
                                        const std::string& version) {
    if (!initialized_) {
        LIZ_ERROR("AssetManager: not initialized — cannot create asset");
        return {};
    }

    auto asset = std::make_unique<Asset>(name, type, size_bytes, version);
    auto uuid = asset->uuid();

    if (!database_.insert(std::move(asset))) {
        return {};
    }

    ++total_created_;

    std::ostringstream msg;
    msg << "Asset created: " << name
        << " (type=" << asset_type_to_string(type)
        << ", version=" << version << ")";
    publish_event(EventType::AssetCreated, msg.str());

    return AssetHandle(uuid, name, type);
}

bool AssetManager::destroy_asset(const std::string& uuid) {
    if (!initialized_) {
        LIZ_ERROR("AssetManager: not initialized — cannot destroy asset");
        return false;
    }

    auto* asset = database_.find(uuid);
    if (!asset) {
        std::ostringstream oss;
        oss << "AssetManager: destroy failed — UUID '" << uuid << "' not found";
        LIZ_WARN(oss.str());
        return false;
    }

    std::string name = asset->name();

    if (!database_.remove(uuid)) {
        return false;
    }

    ++total_destroyed_;

    std::ostringstream msg;
    msg << "Asset destroyed: " << name;
    publish_event(EventType::AssetDestroyed, msg.str());

    return true;
}

// ── Loading / Unloading ──────────────────────────────────────────────────

bool AssetManager::load(const std::string& uuid) {
    if (!initialized_) {
        LIZ_ERROR("AssetManager: not initialized — cannot load asset");
        return false;
    }

    auto* asset = database_.find(uuid);
    if (!asset) {
        std::ostringstream oss;
        oss << "AssetManager: load failed — UUID '" << uuid << "' not found";
        LIZ_ERROR(oss.str());
        return false;
    }

    if (asset->state() != AssetState::Unloaded) {
        std::ostringstream oss;
        oss << "AssetManager: cannot load '" << asset->name()
            << "' — current state=" << asset_state_to_string(asset->state());
        LIZ_WARN(oss.str());
        return false;
    }

    // Simulate: Unloaded -> Loading -> Loaded
    asset->set_state(AssetState::Loading);
    // Simulated load delay (no actual I/O)
    asset->set_state(AssetState::Loaded);
    ++total_loaded_;

    std::ostringstream msg;
    msg << "Asset loaded: " << asset->name();
    publish_event(EventType::AssetLoaded, msg.str());

    return true;
}

bool AssetManager::unload(const std::string& uuid) {
    if (!initialized_) {
        LIZ_ERROR("AssetManager: not initialized — cannot unload asset");
        return false;
    }

    auto* asset = database_.find(uuid);
    if (!asset) {
        std::ostringstream oss;
        oss << "AssetManager: unload failed — UUID '" << uuid << "' not found";
        LIZ_ERROR(oss.str());
        return false;
    }

    if (asset->state() != AssetState::Loaded) {
        std::ostringstream oss;
        oss << "AssetManager: cannot unload '" << asset->name()
            << "' — current state=" << asset_state_to_string(asset->state());
        LIZ_WARN(oss.str());
        return false;
    }

    // Simulate: Loaded -> Unloading -> Unloaded
    asset->set_state(AssetState::Unloading);
    asset->set_state(AssetState::Unloaded);
    ++total_unloaded_;

    std::ostringstream msg;
    msg << "Asset unloaded: " << asset->name();
    publish_event(EventType::AssetUnloaded, msg.str());

    return true;
}

bool AssetManager::reload(const std::string& uuid) {
    if (!initialized_) {
        LIZ_ERROR("AssetManager: not initialized — cannot reload asset");
        return false;
    }

    auto* asset = database_.find(uuid);
    if (!asset) {
        std::ostringstream oss;
        oss << "AssetManager: reload failed — UUID '" << uuid << "' not found";
        LIZ_ERROR(oss.str());
        return false;
    }

    if (asset->state() != AssetState::Loaded) {
        std::ostringstream oss;
        oss << "AssetManager: cannot reload '" << asset->name()
            << "' — must be Loaded, current="
            << asset_state_to_string(asset->state());
        LIZ_WARN(oss.str());
        return false;
    }

    // Simulate: Loaded -> Unloading -> Unloaded -> Loading -> Loaded
    asset->set_state(AssetState::Unloading);
    asset->set_state(AssetState::Unloaded);
    asset->set_state(AssetState::Loading);
    asset->set_state(AssetState::Loaded);
    ++total_reloaded_;

    std::ostringstream msg;
    msg << "Asset reloaded: " << asset->name();
    LIZ_INFO(msg.str());

    return true;
}

// ── Query ─────────────────────────────────────────────────────────────────

Asset* AssetManager::find(const std::string& uuid) const {
    return database_.find(uuid);
}

Asset* AssetManager::find_by_name(const std::string& name) const {
    return database_.find_by_name(name);
}

bool AssetManager::exists(const std::string& uuid) const {
    return database_.exists(uuid);
}

AssetHandle AssetManager::handle(const std::string& uuid) const {
    auto* asset = database_.find(uuid);
    if (!asset) {
        return {};
    }
    return AssetHandle(uuid, asset->name(), asset->type());
}

std::vector<std::string> AssetManager::list() const {
    return database_.list();
}

std::size_t AssetManager::count() const {
    return database_.count();
}

// ── Statistics ──────────────────────────────────────────────────────────────

AssetManagerStats AssetManager::statistics() const {
    AssetManagerStats stats;
    stats.total_created   = total_created_;
    stats.total_destroyed = total_destroyed_;
    stats.total_loaded    = total_loaded_;
    stats.total_unloaded  = total_unloaded_;
    stats.total_reloaded  = total_reloaded_;
    stats.active_assets   = database_.count();

    // Sum memory of all assets.
    auto names = database_.list();
    std::size_t total_mem = 0;
    for (const auto& n : names) {
        auto* a = database_.find_by_name(n);
        if (a) {
            total_mem += a->size_bytes();
        }
    }
    stats.memory_bytes = total_mem;

    return stats;
}

void AssetManager::log_statistics() const {
    auto stats = statistics();
    std::ostringstream oss;
    oss << "AssetManager stats: "
        << "created=" << stats.total_created
        << ", destroyed=" << stats.total_destroyed
        << ", loaded=" << stats.total_loaded
        << ", unloaded=" << stats.total_unloaded
        << ", reloaded=" << stats.total_reloaded
        << ", active=" << stats.active_assets
        << ", memory=" << stats.memory_bytes << " bytes";
    LIZ_INFO(oss.str());
}

// ── Bulk ──────────────────────────────────────────────────────────────────

void AssetManager::clear() {
    database_.clear();
    LIZ_INFO("AssetManager: cleared all assets");
}

// ── EventBus ───────────────────────────────────────────────────────────────

void AssetManager::set_event_bus(EventBus* bus) {
    event_bus_ = bus;
}

void AssetManager::publish_event(EventType type, const std::string& message) {
    if (event_bus_) {
        event_bus_->publish(Event(type, "AssetManager", message));
    }
}

} // namespace liz
