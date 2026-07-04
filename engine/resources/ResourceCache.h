#pragma once

#include "engine/resources/Resource.h"
#include "engine/resources/ResourceHandle.h"

#include <cstddef>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace liz {

/// Cache statistics snapshot.
struct ResourceCacheStats {
    std::size_t total_resources  = 0;
    std::size_t total_bytes      = 0;
    std::size_t orphans          = 0;
    std::size_t by_type[10]      = {};  // indexed by ResourceType
};

/// In-memory cache for managed resources.
///
/// Stores resources by UUID.  No persistence, no file I/O.
class ResourceCache {
public:
    ResourceCache() = default;
    ~ResourceCache() = default;

    // Non-copyable.
    ResourceCache(const ResourceCache&) = delete;
    ResourceCache& operator=(const ResourceCache&) = delete;

    /// Add a resource to the cache.  Returns false if UUID already exists.
    bool add(std::shared_ptr<Resource> resource);

    /// Remove a resource by UUID.  Returns the removed resource, or nullptr.
    std::shared_ptr<Resource> remove(const std::string& uuid);

    /// Find a resource by UUID.  Returns nullptr if not found.
    std::shared_ptr<Resource> find(const std::string& uuid) const;

    /// Find a resource by name.  Returns nullptr if not found.
    std::shared_ptr<Resource> find_by_name(const std::string& name) const;

    /// Check whether a resource with the given UUID exists.
    bool exists(const std::string& uuid) const;

    /// Remove all resources from the cache.
    void clear();

    /// Snapshot of cache statistics.
    ResourceCacheStats statistics() const;

    /// Log statistics via LIZ_INFO.
    void log_statistics() const;

    /// Number of resources in the cache.
    std::size_t size() const;

    /// Get all resource UUIDs.
    std::vector<std::string> all_uuids() const;

private:
    std::unordered_map<std::string, std::shared_ptr<Resource>> cache_;
};

} // namespace liz