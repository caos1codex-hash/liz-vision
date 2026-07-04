#pragma once

#include "engine/resources/Resource.h"

#include <string>

namespace liz {

class Resource;

/// Lightweight handle to a managed Resource.
///
/// Other modules use this instead of raw pointers or shared_ptr.
/// The handle stores only the resource UUID and type — it does NOT
/// own or directly access the Resource object.
///
/// To obtain the actual Resource, go through ResourceManager.
class ResourceHandle {
public:
    ResourceHandle() = default;

    /// Create a handle to the resource with the given UUID and type.
    ResourceHandle(std::string uuid, std::string name, ResourceType type);

    // -- Query -----------------------------------------------------------------
    bool        valid() const;
    const std::string& uuid() const;
    const std::string& name() const;
    ResourceType type() const;

    /// One-line summary.
    std::string info() const;

private:
    std::string  uuid_;
    std::string  name_;
    ResourceType type_ = ResourceType::Unknown;
};

} // namespace liz