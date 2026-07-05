#pragma once

#include <string>
#include <vector>

namespace liz {

/// Manifest describing a plugin's metadata and dependencies.
///
/// This is the "package.json" equivalent for a plugin.
/// Fully in-memory — no filesystem parsing.
class PluginManifest {
public:
    PluginManifest() = default;

    // ── Accessors ────────────────────────────────────────────────────────

    const std::string& name() const { return name_; }
    const std::string& version() const { return version_; }
    const std::string& author() const { return author_; }
    const std::string& website() const { return website_; }
    const std::string& license_name() const { return license_name_; }
    const std::string& description() const { return description_; }
    const std::string& required_engine_version() const { return required_engine_version_; }
    const std::string& api_version() const { return api_version_; }

    const std::vector<std::string>& dependencies() const { return dependencies_; }

    // ── Mutators ─────────────────────────────────────────────────────────

    void set_name(const std::string& name);
    void set_version(const std::string& version);
    void set_author(const std::string& author);
    void set_website(const std::string& website);
    void set_license(const std::string& license_name);
    void set_description(const std::string& desc);
    void set_required_engine_version(const std::string& ver);
    void set_api_version(const std::string& ver);
    void add_dependency(const std::string& dep);

    /// Generate a formatted summary string.
    std::string to_string() const;

private:
    std::string name_;
    std::string version_;
    std::string author_;
    std::string website_;
    std::string license_name_;
    std::string description_;
    std::string required_engine_version_;
    std::string api_version_;
    std::vector<std::string> dependencies_;
};

} // namespace liz
