#pragma once

#include <string>

namespace liz {

/// API version information.
///
/// Follows Semantic Versioning (major.minor.patch) with an additional
/// build number for internal tracking.
class ApiVersion {
public:
    constexpr ApiVersion() = default;

    constexpr ApiVersion(int major, int minor, int patch, int build = 0)
        : major_(major), minor_(minor), patch_(patch), build_(build) {}

    // ── Accessors ───────────────────────────────────────────────────────

    constexpr int major() const { return major_; }
    constexpr int minor() const { return minor_; }
    constexpr int patch() const { return patch_; }
    constexpr int build() const { return build_; }

    // ── String representation ──────────────────────────────────────────

    /// Returns the version as "major.minor.patch" (e.g. "1.0.0").
    std::string version_string() const;

    /// Returns the full version with build (e.g. "1.0.0+42").
    std::string full_version_string() const;

    // ── Compatibility ─────────────────────────────────────────────────

    /// Check compatibility with another version.
    /// Two versions are compatible if they share the same major number.
    bool compatible_with(const ApiVersion& other) const;

    // ── Comparison ─────────────────────────────────────────────────────

    bool operator==(const ApiVersion& other) const;
    bool operator!=(const ApiVersion& other) const;
    bool operator<(const ApiVersion& other) const;

private:
    int major_ = 1;
    int minor_ = 0;
    int patch_ = 0;
    int build_ = 0;
};

} // namespace liz
