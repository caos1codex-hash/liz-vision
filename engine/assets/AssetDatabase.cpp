#include "engine/assets/AssetDatabase.h"
#include "engine/core/Logger.h"

#include <algorithm>
#include <sstream>

namespace liz {

// ── Constructor ───────────────────────────────────────────────────────────

AssetDatabase::AssetDatabase() = default;

// ── CRUD ──────────────────────────────────────────────────────────────────

bool AssetDatabase::insert(std::unique_ptr<Asset> asset) {
    if (!asset) {
        LIZ_WARN("AssetDatabase: cannot insert null asset");
        return false;
    }

    // Check for duplicate UUID.
    if (exists(asset->uuid())) {
        std::ostringstream oss;
        oss << "AssetDatabase: duplicate UUID rejected — '" << asset->name() << "'";
        LIZ_WARN(oss.str());
        return false;
    }

    std::ostringstream oss;
    oss << "AssetDatabase: inserted '" << asset->name()
        << "' (type=" << asset_type_to_string(asset->type()) << ")";
    LIZ_INFO(oss.str());

    assets_.push_back(std::move(asset));
    ++total_inserted_;
    return true;
}

bool AssetDatabase::remove(const std::string& uuid) {
    auto it = std::find_if(assets_.begin(), assets_.end(),
        [&uuid](const std::unique_ptr<Asset>& a) { return a->uuid() == uuid; });

    if (it == assets_.end()) {
        std::ostringstream oss;
        oss << "AssetDatabase: remove failed — UUID '" << uuid << "' not found";
        LIZ_WARN(oss.str());
        return false;
    }

    std::ostringstream oss;
    oss << "AssetDatabase: removed '" << (*it)->name() << "'";
    LIZ_INFO(oss.str());

    assets_.erase(it);
    ++total_removed_;
    return true;
}

// ── Query ─────────────────────────────────────────────────────────────────

Asset* AssetDatabase::find(const std::string& uuid) const {
    auto it = std::find_if(assets_.begin(), assets_.end(),
        [&uuid](const std::unique_ptr<Asset>& a) { return a->uuid() == uuid; });

    return (it != assets_.end()) ? it->get() : nullptr;
}

Asset* AssetDatabase::find_by_name(const std::string& name) const {
    auto it = std::find_if(assets_.begin(), assets_.end(),
        [&name](const std::unique_ptr<Asset>& a) { return a->name() == name; });

    return (it != assets_.end()) ? it->get() : nullptr;
}

bool AssetDatabase::exists(const std::string& uuid) const {
    return find(uuid) != nullptr;
}

std::vector<std::string> AssetDatabase::list() const {
    std::vector<std::string> names;
    names.reserve(assets_.size());
    for (const auto& asset : assets_) {
        names.push_back(asset->name());
    }
    return names;
}

std::size_t AssetDatabase::count() const {
    return assets_.size();
}

// ── Bulk ──────────────────────────────────────────────────────────────────

void AssetDatabase::clear() {
    std::size_t cleared = assets_.size();
    assets_.clear();
    total_removed_ += cleared;

    std::ostringstream oss;
    oss << "AssetDatabase: cleared " << cleared << " assets";
    LIZ_INFO(oss.str());
}

// ── Statistics ───────────────────────────────────────────────────────────

AssetDatabaseStats AssetDatabase::statistics() const {
    return AssetDatabaseStats{
        total_inserted_,
        total_removed_,
        assets_.size()
    };
}

void AssetDatabase::log_statistics() const {
    auto stats = statistics();
    std::ostringstream oss;
    oss << "AssetDatabase stats: "
        << "inserted=" << stats.total_inserted
        << ", removed=" << stats.total_removed
        << ", active=" << stats.total_assets;
    LIZ_INFO(oss.str());
}

} // namespace liz
