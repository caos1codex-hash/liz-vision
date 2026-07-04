#include "engine/assets/AssetHandle.h"

#include <sstream>

namespace liz {

// ── Constructor ───────────────────────────────────────────────────────────

AssetHandle::AssetHandle(std::string uuid, std::string name, AssetType type)
    : uuid_(std::move(uuid))
    , name_(std::move(name))
    , type_(type) {}

// ── Query ─────────────────────────────────────────────────────────────────

bool AssetHandle::valid() const {
    return !uuid_.empty();
}

const std::string& AssetHandle::uuid() const { return uuid_; }
const std::string& AssetHandle::name() const { return name_; }
AssetType          AssetHandle::type() const { return type_; }

// ── Reference semantics ────────────────────────────────────────────────────

void AssetHandle::acquire() {
    // Actual ref counting happens through AssetManager.
    // This is a placeholder for the handle-level semantic.
}

bool AssetHandle::release() {
    // Actual ref counting happens through AssetManager.
    return true;
}

// ── Info ──────────────────────────────────────────────────────────────────

std::string AssetHandle::info() const {
    std::ostringstream oss;
    oss << "AssetHandle[" << name_
        << "] type=" << asset_type_to_string(type_)
        << " uuid=" << uuid_
        << " valid=" << (valid() ? "true" : "false");
    return oss.str();
}

} // namespace liz
