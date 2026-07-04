#include "engine/resources/Resource.h"

#include <chrono>
#include <iomanip>
#include <sstream>

namespace liz {

// -- Static counter for UUID generation ----------------------------------------

std::uint64_t Resource::uuid_counter_ = 1;

// -- Enum converters -----------------------------------------------------------

const char* resource_type_to_string(ResourceType type) {
    switch (type) {
        case ResourceType::Video:        return "Video";
        case ResourceType::Frame:        return "Frame";
        case ResourceType::Tensor:       return "Tensor";
        case ResourceType::AIModel:      return "AIModel";
        case ResourceType::GPUBuffer:    return "GPUBuffer";
        case ResourceType::Plugin:       return "Plugin";
        case ResourceType::Configuration:return "Configuration";
        case ResourceType::Project:      return "Project";
        case ResourceType::Temporary:    return "Temporary";
        case ResourceType::Unknown:      return "Unknown";
    }
    return "Unknown";
}

const char* resource_state_to_string(ResourceState state) {
    switch (state) {
        case ResourceState::Loaded:   return "Loaded";
        case ResourceState::Unloaded: return "Unloaded";
        case ResourceState::Error:    return "Error";
    }
    return "Unknown";
}

// -- Helpers -------------------------------------------------------------------

std::string Resource::generate_uuid() {
    auto id = uuid_counter_++;
    std::ostringstream oss;
    oss << std::hex << std::setfill('0') << std::setw(8) << (id & 0xFFFFFFFFULL);
    return oss.str();
}

double Resource::now() {
    return std::chrono::duration<double>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
}

// -- Constructor ---------------------------------------------------------------

Resource::Resource(std::string name, ResourceType type)
    : uuid_(generate_uuid()),
      name_(std::move(name)),
      type_(type),
      created_at_(now()),
      last_accessed_(created_at_) {}

Resource::Resource(std::string name, ResourceType type, std::size_t size_bytes)
    : uuid_(generate_uuid()),
      name_(std::move(name)),
      type_(type),
      size_bytes_(size_bytes),
      created_at_(now()),
      last_accessed_(created_at_) {}

// -- Identity ------------------------------------------------------------------

const std::string& Resource::uuid() const              { return uuid_; }
const std::string& Resource::name() const              { return name_; }
void               Resource::set_name(const std::string& n) { name_ = n; }
ResourceType       Resource::type() const              { return type_; }

// -- Size ----------------------------------------------------------------------

std::size_t Resource::size_bytes() const { return size_bytes_; }
void        Resource::set_size(std::size_t bytes) { size_bytes_ = bytes; }

// -- State ---------------------------------------------------------------------

ResourceState Resource::state() const          { return state_; }
void          Resource::set_state(ResourceState s) { state_ = s; }

// -- Reference counting --------------------------------------------------------

std::uint32_t Resource::ref_count() const { return ref_count_; }

void Resource::add_ref()  { ++ref_count_; }

void Resource::release() {
    if (ref_count_ > 0) --ref_count_;
}

bool Resource::is_orphan() const { return ref_count_ == 0; }

// -- Timestamps ----------------------------------------------------------------

double Resource::created_at() const     { return created_at_; }
double Resource::last_accessed() const { return last_accessed_; }

void Resource::touch() { last_accessed_ = now(); }

// -- Info ----------------------------------------------------------------------

std::string Resource::info() const {
    std::ostringstream oss;
    oss << "Resource{uuid=" << uuid_
        << " name='" << name_ << "'"
        << " type=" << resource_type_to_string(type_)
        << " size=" << size_bytes_
        << " state=" << resource_state_to_string(state_)
        << " refs=" << ref_count_ << "}";
    return oss.str();
}

} // namespace liz