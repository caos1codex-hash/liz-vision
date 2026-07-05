#include "engine/pluginloader/PluginDescriptor.h"
#include "engine/core/Logger.h"

#include <chrono>
#include <random>
#include <sstream>

namespace liz {

// ── UUID generation ────────────────────────────────────────────────────────

std::string PluginDescriptor::generate_uuid() {
    auto now = std::chrono::steady_clock::now().time_since_epoch().count();
    std::mt19937_64 rng(static_cast<std::mt19937_64::result_type>(now));
    std::uniform_int_distribution<std::uint64_t> dist(0, 0xFFFFFFFFFFFFFFFF);
    auto val = dist(rng);
    std::ostringstream oss;
    oss << std::hex << (val & 0xFFFFFFFF);
    std::string hex = oss.str();
    if (hex.length() < 8) {
        hex.insert(0, 8 - hex.length(), '0');
    }
    return hex.substr(0, 8);
}

// ── Constructors ───────────────────────────────────────────────────────────

PluginDescriptor::PluginDescriptor()
    : uuid_(generate_uuid())
    , category_(PluginCategory::Unknown)
    , enabled_(true)
    , loaded_(false)
    , load_time_ms_(0.0)
    , unload_time_ms_(0.0)
{}

PluginDescriptor::PluginDescriptor(const std::string& name,
                                   const std::string& author,
                                   const std::string& version,
                                   PluginCategory category)
    : uuid_(generate_uuid())
    , name_(name)
    , author_(author)
    , version_(version)
    , category_(category)
    , enabled_(true)
    , loaded_(false)
    , load_time_ms_(0.0)
    , unload_time_ms_(0.0)
{}

// ── Move semantics ────────────────────────────────────────────────────────

PluginDescriptor::PluginDescriptor(PluginDescriptor&& other) noexcept
    : uuid_(std::move(other.uuid_))
    , name_(std::move(other.name_))
    , author_(std::move(other.author_))
    , version_(std::move(other.version_))
    , description_(std::move(other.description_))
    , category_(other.category_)
    , api_version_(std::move(other.api_version_))
    , engine_version_(std::move(other.engine_version_))
    , enabled_(other.enabled_)
    , loaded_(other.loaded_)
    , load_time_ms_(other.load_time_ms_)
    , unload_time_ms_(other.unload_time_ms_)
{
    other.enabled_ = false;
    other.loaded_ = false;
    other.load_time_ms_ = 0.0;
    other.unload_time_ms_ = 0.0;
}

PluginDescriptor& PluginDescriptor::operator=(PluginDescriptor&& other) noexcept {
    if (this != &other) {
        uuid_ = std::move(other.uuid_);
        name_ = std::move(other.name_);
        author_ = std::move(other.author_);
        version_ = std::move(other.version_);
        description_ = std::move(other.description_);
        category_ = other.category_;
        api_version_ = std::move(other.api_version_);
        engine_version_ = std::move(other.engine_version_);
        enabled_ = other.enabled_;
        loaded_ = other.loaded_;
        load_time_ms_ = other.load_time_ms_;
        unload_time_ms_ = other.unload_time_ms_;
        other.enabled_ = false;
        other.loaded_ = false;
        other.load_time_ms_ = 0.0;
        other.unload_time_ms_ = 0.0;
    }
    return *this;
}

// ── Mutators ──────────────────────────────────────────────────────────────

void PluginDescriptor::set_enabled(bool enabled) { enabled_ = enabled; }
void PluginDescriptor::set_loaded(bool loaded)   { loaded_ = loaded; }
void PluginDescriptor::set_load_time(double ms)   { load_time_ms_ = ms; }
void PluginDescriptor::set_unload_time(double ms) { unload_time_ms_ = ms; }

void PluginDescriptor::set_description(const std::string& desc) {
    description_ = desc;
}

void PluginDescriptor::set_api_version(const std::string& ver) {
    api_version_ = ver;
}

void PluginDescriptor::set_engine_version(const std::string& ver) {
    engine_version_ = ver;
}

// ── to_string ────────────────────────────────────────────────────────────

std::string PluginDescriptor::to_string() const {
    std::ostringstream oss;
    oss << "Plugin["
        << "uuid=" << uuid_
        << ", name=\"" << name_ << "\""
        << ", author=\"" << author_ << "\""
        << ", version=" << version_
        << ", category=" << plugin_category_to_string(category_)
        << ", enabled=" << (enabled_ ? "true" : "false")
        << ", loaded=" << (loaded_ ? "true" : "false");
    if (load_time_ms_ > 0.0) {
        oss << ", load_time=" << load_time_ms_ << "ms";
    }
    oss << "]";
    return oss.str();
}

} // namespace liz
