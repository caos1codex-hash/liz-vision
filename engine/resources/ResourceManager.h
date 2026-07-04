#pragma once

#include "engine/resources/ResourceCache.h"
#include "engine/resources/ResourceHandle.h"

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

namespace liz {

/// Resource manager statistics snapshot.
struct ResourceManagerStats {
    std::size_t total_created  = 0;
    std::size_t total_destroyed = 0;
    std::size_t total_handles   = 0;
};

/// Single point of access for all engine resources.
///
/// Creates, destroys, finds, and tracks every resource.
/// All modules should go through this — never create resources "por su cuenta".
class ResourceManager {
public:
    ResourceManager();
    ~ResourceManager() = default;

    // Non-copyable.
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;

    // -- Creation --------------------------------------------------------------

    /// Create a new resource and register it in the cache.
    /// Returns a lightweight handle.
    ResourceHandle create(const std::string& name, ResourceType type);

    /// Create a new resource with an explicit size.
    ResourceHandle create(const std::string& name, ResourceType type,
                          std::size_t size_bytes);

    // -- Destruction -----------------------------------------------------------

    /// Remove a resource by handle.  Returns true if found and removed.
    bool destroy(const ResourceHandle& handle);

    /// Remove a resource by UUID.  Returns true if found and removed.
    bool destroy(const std::string& uuid);

    // -- Query -----------------------------------------------------------------

    /// Get the underlying resource (null if not found).
    std::shared_ptr<Resource> get(const ResourceHandle& handle) const;

    /// Get the underlying resource by UUID (null if not found).
    std::shared_ptr<Resource> get(const std::string& uuid) const;

    /// Get a handle for an existing resource by name.
    ResourceHandle find_by_name(const std::string& name) const;

    /// Check if a resource exists by handle.
    bool exists(const ResourceHandle& handle) const;

    /// Check if a resource exists by UUID.
    bool exists(const std::string& uuid) const;

    // -- Reference counting (through handles) -----------------------------------

    /// Increment the reference count of a resource.
    bool add_ref(const ResourceHandle& handle);

    /// Decrement the reference count.  Returns true if still alive.
    bool release(const ResourceHandle& handle);

    // -- Bulk operations -------------------------------------------------------

    /// Remove all resources from the cache.
    void clear_cache();

    // -- Statistics ------------------------------------------------------------

    ResourceManagerStats stats() const;

    void log_statistics() const;

    /// Access the underlying cache (read-only).
    const ResourceCache& cache() const;

    std::size_t resource_count() const;

private:
    ResourceCache        cache_;
    std::size_t          total_created_   = 0;
    std::size_t          total_destroyed_ = 0;
};

} // namespace liz