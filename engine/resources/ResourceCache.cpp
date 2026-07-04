#include "engine/resources/ResourceCache.h"
#include "engine/core/Logger.h"

#include <algorithm>
#include <sstream>

namespace liz {

// -- Add / Remove / Find -------------------------------------------------------

bool ResourceCache::add(std::shared_ptr<Resource> resource) {
    if (!resource) return false;

    const auto& uuid = resource->uuid();
    if (cache_.find(uuid) != cache_.end()) {
        std::ostringstream oss;
        oss << "ResourceCache: duplicate UUID " << uuid << " — rejected";
        LIZ_WARN(oss.str());
        return false;
    }

    cache_[uuid] = std::move(resource);
    return true;
}

std::shared_ptr<Resource> ResourceCache::remove(const std::string& uuid) {
    auto it = cache_.find(uuid);
    if (it == cache_.end()) return nullptr;

    auto resource = std::move(it->second);
    cache_.erase(it);

    std::ostringstream oss;
    oss << "ResourceCache: removed " << resource->info();
    LIZ_DEBUG(oss.str());

    return resource;
}

std::shared_ptr<Resource> ResourceCache::find(const std::string& uuid) const {
    auto it = cache_.find(uuid);
    if (it != cache_.end()) {
        const_cast<Resource&>(*it->second).touch();  // update last accessed
        return it->second;
    }
    return nullptr;
}

std::shared_ptr<Resource> ResourceCache::find_by_name(const std::string& name) const {
    for (const auto& [uuid, res] : cache_) {
        if (res->name() == name) {
            const_cast<Resource&>(*res).touch();
            return res;
        }
    }
    return nullptr;
}

bool ResourceCache::exists(const std::string& uuid) const {
    return cache_.find(uuid) != cache_.end();
}

// -- Clear --------------------------------------------------------------------

void ResourceCache::clear() {
    std::ostringstream oss;
    oss << "ResourceCache: clearing " << cache_.size() << " resource(s)";
    LIZ_INFO(oss.str());
    cache_.clear();
}

// -- Query --------------------------------------------------------------------

std::size_t ResourceCache::size() const {
    return cache_.size();
}

std::vector<std::string> ResourceCache::all_uuids() const {
    std::vector<std::string> uuids;
    uuids.reserve(cache_.size());
    for (const auto& [uuid, _] : cache_) {
        uuids.push_back(uuid);
    }
    return uuids;
}

// -- Statistics ----------------------------------------------------------------

ResourceCacheStats ResourceCache::statistics() const {
    ResourceCacheStats stats;
    stats.total_resources = cache_.size();

    for (const auto& [uuid, res] : cache_) {
        stats.total_bytes += res->size_bytes();
        if (res->is_orphan()) ++stats.orphans;

        auto idx = static_cast<std::size_t>(res->type());
        if (idx < 10) ++stats.by_type[idx];
    }

    return stats;
}

void ResourceCache::log_statistics() const {
    auto s = statistics();

    std::ostringstream oss;
    oss << "ResourceCache: "
        << s.total_resources << " resources, "
        << (s.total_bytes / 1024) << "KB total, "
        << s.orphans << " orphans";
    LIZ_INFO(oss.str());

    // Per-type breakdown.
    for (int i = 0; i < 10; ++i) {
        auto count = s.by_type[i];
        if (count > 0) {
            std::ostringstream oss2;
            oss2 << "  " << resource_type_to_string(static_cast<ResourceType>(i))
                 << ": " << count;
            LIZ_INFO(oss2.str());
        }
    }
}

} // namespace liz