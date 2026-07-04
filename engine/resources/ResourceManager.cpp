#include "engine/resources/ResourceManager.h"
#include "engine/core/Logger.h"

#include <sstream>

namespace liz {

// -- Constructor --------------------------------------------------------------

ResourceManager::ResourceManager() {
    LIZ_INFO("ResourceManager: initialized");
}

// -- Creation ------------------------------------------------------------------

ResourceHandle ResourceManager::create(const std::string& name, ResourceType type) {
    auto resource = std::make_shared<Resource>(name, type);
    const auto& uuid = resource->uuid();

    if (!cache_.add(resource)) {
        std::ostringstream oss;
        oss << "ResourceManager: failed to create resource '" << name << "'";
        LIZ_ERROR(oss.str());
        return {};
    }

    ++total_created_;

    std::ostringstream oss;
    oss << "ResourceManager: created " << resource->info();
    LIZ_INFO(oss.str());

    return ResourceHandle{uuid, name, type};
}

ResourceHandle ResourceManager::create(const std::string& name, ResourceType type,
                                        std::size_t size_bytes) {
    auto resource = std::make_shared<Resource>(name, type, size_bytes);
    const auto& uuid = resource->uuid();

    if (!cache_.add(resource)) {
        std::ostringstream oss;
        oss << "ResourceManager: failed to create resource '" << name << "'";
        LIZ_ERROR(oss.str());
        return {};
    }

    ++total_created_;

    std::ostringstream oss;
    oss << "ResourceManager: created " << resource->info();
    LIZ_INFO(oss.str());

    return ResourceHandle{uuid, name, type};
}

// -- Destruction ---------------------------------------------------------------

bool ResourceManager::destroy(const ResourceHandle& handle) {
    if (!handle.valid()) return false;
    return destroy(handle.uuid());
}

bool ResourceManager::destroy(const std::string& uuid) {
    auto removed = cache_.remove(uuid);
    if (removed) {
        ++total_destroyed_;
        std::ostringstream oss;
        oss << "ResourceManager: destroyed resource " << uuid;
        LIZ_INFO(oss.str());
        return true;
    }
    return false;
}

// -- Query ---------------------------------------------------------------------

std::shared_ptr<Resource> ResourceManager::get(const ResourceHandle& handle) const {
    if (!handle.valid()) return nullptr;
    return cache_.find(handle.uuid());
}

std::shared_ptr<Resource> ResourceManager::get(const std::string& uuid) const {
    return cache_.find(uuid);
}

ResourceHandle ResourceManager::find_by_name(const std::string& name) const {
    auto res = cache_.find_by_name(name);
    if (!res) return {};
    return ResourceHandle{res->uuid(), res->name(), res->type()};
}

bool ResourceManager::exists(const ResourceHandle& handle) const {
    if (!handle.valid()) return false;
    return cache_.exists(handle.uuid());
}

bool ResourceManager::exists(const std::string& uuid) const {
    return cache_.exists(uuid);
}

// -- Reference counting --------------------------------------------------------

bool ResourceManager::add_ref(const ResourceHandle& handle) {
    auto res = get(handle);
    if (!res) return false;
    res->add_ref();
    return true;
}

bool ResourceManager::release(const ResourceHandle& handle) {
    auto res = get(handle);
    if (!res) return false;
    res->release();
    return true;
}

// -- Bulk operations -----------------------------------------------------------

void ResourceManager::clear_cache() {
    std::ostringstream oss;
    oss << "ResourceManager: clearing cache ("
        << cache_.size() << " resources)";
    LIZ_INFO(oss.str());
    cache_.clear();
}

// -- Statistics ----------------------------------------------------------------

ResourceManagerStats ResourceManager::stats() const {
    return { total_created_, total_destroyed_, cache_.size() };
}

void ResourceManager::log_statistics() const {
    auto s = stats();
    std::ostringstream oss;
    oss << "ResourceManager: created=" << s.total_created
        << " destroyed=" << s.total_destroyed
        << " active=" << s.total_handles;
    LIZ_INFO(oss.str());
    cache_.log_statistics();
}

const ResourceCache& ResourceManager::cache() const { return cache_; }
std::size_t ResourceManager::resource_count() const { return cache_.size(); }

} // namespace liz