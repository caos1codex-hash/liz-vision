#pragma once

#include <string>

namespace liz {

/// Execution profiles for the Advanced Configuration System.
///
/// A profile is a named preset of configuration values that tunes the
/// engine for a particular scenario (development ergonomics, production
/// safety, benchmark throughput).
enum class ConfigurationProfile {
    Development,
    Production,
    Benchmark,
    Custom
};

/// Convert a ConfigurationProfile to a human-readable string.
inline const char* configuration_profile_to_string(ConfigurationProfile profile) {
    switch (profile) {
        case ConfigurationProfile::Development: return "Development";
        case ConfigurationProfile::Production:  return "Production";
        case ConfigurationProfile::Benchmark:   return "Benchmark";
        case ConfigurationProfile::Custom:       return "Custom";
    }
    return "Unknown";
}

/// Parse a profile name string into a ConfigurationProfile.
/// Returns Custom when no known name matches.
inline ConfigurationProfile configuration_profile_from_string(const std::string& name) {
    if (name == "Development") return ConfigurationProfile::Development;
    if (name == "Production")  return ConfigurationProfile::Production;
    if (name == "Benchmark")   return ConfigurationProfile::Benchmark;
    return ConfigurationProfile::Custom;
}

} // namespace liz
