#include "engine/pluginloader/PluginManifest.h"

#include <sstream>

namespace liz {

// ── Mutators ──────────────────────────────────────────────────────────────

void PluginManifest::set_name(const std::string& name)              { name_ = name; }
void PluginManifest::set_version(const std::string& version)        { version_ = version; }
void PluginManifest::set_author(const std::string& author)          { author_ = author; }
void PluginManifest::set_website(const std::string& website)        { website_ = website; }
void PluginManifest::set_license(const std::string& license_name)   { license_name_ = license_name; }
void PluginManifest::set_description(const std::string& desc)       { description_ = desc; }
void PluginManifest::set_required_engine_version(const std::string& ver) { required_engine_version_ = ver; }
void PluginManifest::set_api_version(const std::string& ver)        { api_version_ = ver; }

void PluginManifest::add_dependency(const std::string& dep) {
    dependencies_.push_back(dep);
}

// ── to_string ────────────────────────────────────────────────────────────

std::string PluginManifest::to_string() const {
    std::ostringstream oss;
    oss << "Manifest["
        << "name=\"" << name_ << "\""
        << ", version=" << version_
        << ", author=\"" << author_ << "\"";
    if (!website_.empty()) {
        oss << ", website=\"" << website_ << "\"";
    }
    if (!license_name_.empty()) {
        oss << ", license=\"" << license_name_ << "\"";
    }
    if (!dependencies_.empty()) {
        oss << ", deps=[";
        for (std::size_t i = 0; i < dependencies_.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << "\"" << dependencies_[i] << "\"";
        }
        oss << "]";
    }
    if (!required_engine_version_.empty()) {
        oss << ", engine=" << required_engine_version_;
    }
    if (!api_version_.empty()) {
        oss << ", api=" << api_version_;
    }
    oss << "]";
    return oss.str();
}

} // namespace liz
