#pragma once

#include "engine/assets/AssetType.h"

#include <string>

namespace liz {

/// Lightweight handle to a managed Asset.
///
/// Other modules use this instead of raw pointers or shared_ptr.
/// The handle stores only the asset UUID, name, and type — it does NOT
/// own or directly access the Asset object.
///
/// To obtain the actual Asset, go through AssetManager.
class AssetHandle {
public:
    AssetHandle() = default;

    /// Create a handle to the asset with the given UUID, name, and type.
    AssetHandle(std::string uuid, std::string name, AssetType type);

    // ── Query ──────────────────────────────────────────────────────────

    /// True if this handle points to a valid (non-empty UUID) asset.
    bool valid() const;

    const std::string& uuid() const;
    const std::string& name() const;
    AssetType          type() const;

    // ── Reference semantics ──────────────────────────────────────────────

    /// Increment the reference counter (via AssetManager).
    void acquire();

    /// Decrement the reference counter (via AssetManager).
    /// Returns true if the asset is still alive.
    bool release();

    // ── Info ─────────────────────────────────────────────────────────────

    /// One-line summary for logging.
    std::string info() const;

private:
    std::string uuid_;
    std::string name_;
    AssetType   type_ = AssetType::Unknown;
};

} // namespace liz
