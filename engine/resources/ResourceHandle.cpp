#include "engine/resources/ResourceHandle.h"
#include "engine/resources/Resource.h"

#include <sstream>

namespace liz {

ResourceHandle::ResourceHandle(std::string uuid, std::string name, ResourceType type)
    : uuid_(std::move(uuid)),
      name_(std::move(name)),
      type_(type) {}

bool ResourceHandle::valid() const { return !uuid_.empty(); }

const std::string& ResourceHandle::uuid() const { return uuid_; }
const std::string& ResourceHandle::name() const { return name_; }
ResourceType       ResourceHandle::type() const { return type_; }

std::string ResourceHandle::info() const {
    std::ostringstream oss;
    oss << "Handle{uuid=" << uuid_
        << " name='" << name_ << "'"
        << " type=" << resource_type_to_string(type_) << "}";
    return oss.str();
}

} // namespace liz