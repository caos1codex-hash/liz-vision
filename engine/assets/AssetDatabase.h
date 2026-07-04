#pragma once

#include "engine/assets/Asset.h"

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

namespace liz {

/// Statistics snapshot of the AssetDatabase.
struct AssetDatabaseStats {
    std::size_t total_inserted  = 0;
    std::size_t total_removed   = 0;
    std::size_t total_assets    = 0;
};

/// In-memory database for all engine assets.
///
/// No filesystem, no JSON, no SQLite — purely in-memory storage.
/// Provides CRUD operations and type-based querying.
class AssetDatabase {
public:
    AssetDatabase();
    ~AssetDatabase() = default;

    // Non-copyable.
    AssetDatabase(const AssetDatabase&) = delete;
    AssetDatabase& operator=(const AssetDatabase&) = delete;

    // ── CRUD ──────────────────────────────────────────────────────────────

    /// Insert an asset into the database.  Takes ownership.
    /// Returns true on success.
    bool insert(std::unique_ptr<Asset> asset);

    /// Remove an asset by UUID.  Returns true if found and removed.
    bool remove(const std::string& uuid);

    // ── Query ─────────────────────────────────────────────────────────────

    /// Find an asset by UUID.  Returns nullptr if not found.
    Asset* find(const std::string& uuid) const;

    /// Find an asset by name.  Returns nullptr if not found.
    Asset* find_by_name(const std::string& name) const;

    /// Check if an asset with the given UUID exists.
    bool exists(const std::string& uuid) const;

    /// Get the names of all stored assets.
    std::vector<std::string> list() const;

    /// Number of stored assets.
    std::size_t count() const;

    // ── Bulk ──────────────────────────────────────────────────────────────

    /// Remove all assets from the database.
    void clear();

    // ── Statistics ───────────────────────────────────────────────────────

    /// Get a statistics snapshot.
    AssetDatabaseStats statistics() const;

    /// Log a summary of the database statistics.
    void log_statistics() const;

private:
    std::vector<std::unique_ptr<Asset>> assets_;
    std::size_t total_inserted_ = 0;
    std::size_t total_removed_  = 0;
};

} // namespace liz
