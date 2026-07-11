#pragma once

#include "engine/config/ConfigValue.h"

#include <cstddef>
#include <string>
#include <vector>

namespace liz {

/// Where an override value comes from. Higher priority sources win.
enum class ConfigOverrideSource {
    SchemaDefault, // lowest — fallback default from schema
    Profile,       // values contributed by an activated profile
    Environment,   // LIZ_* environment variables
    Runtime        // explicit runtime override (highest)
};

/// Convert a ConfigOverrideSource to a human-readable string.
inline const char* config_override_source_to_string(ConfigOverrideSource src) {
    switch (src) {
        case ConfigOverrideSource::SchemaDefault: return "SchemaDefault";
        case ConfigOverrideSource::Profile:       return "Profile";
        case ConfigOverrideSource::Environment:   return "Environment";
        case ConfigOverrideSource::Runtime:       return "Runtime";
    }
    return "Unknown";
}

/// Priority of an override source (higher wins).
inline int config_override_source_priority(ConfigOverrideSource src) {
    switch (src) {
        case ConfigOverrideSource::SchemaDefault: return 0;
        case ConfigOverrideSource::Profile:       return 1;
        case ConfigOverrideSource::Environment:   return 2;
        case ConfigOverrideSource::Runtime:       return 3;
    }
    return -1;
}

/// A single runtime override of a configuration key.
class ConfigOverride {
public:
    ConfigOverride(std::string section, std::string key,
                   ConfigValue::ValueVariant value,
                   ConfigOverrideSource source = ConfigOverrideSource::Runtime);

    const std::string& section() const;
    const std::string& key() const;
    const ConfigValue::ValueVariant& value() const;
    ConfigOverrideSource source() const;
    int priority() const;

    std::string to_string() const;

private:
    std::string             section_;
    std::string             key_;
    ConfigValue::ValueVariant value_;
    ConfigOverrideSource    source_;
};

/// Ordered collection of overrides. `resolve()` returns the value of the
/// highest-priority override matching (section,key), or nullptr when none.
class ConfigOverrideStack {
public:
    ConfigOverrideStack() = default;

    /// Add an override (dedup: replaces any same key+source entry).
    void add(ConfigOverride override_entry);

    /// Remove an override by section+key+source. Returns true if removed.
    bool remove(const std::string& section, const std::string& key,
                ConfigOverrideSource source);

    /// Resolve the effective override value for (section,key).
    /// Returns nullptr when no override applies.
    const ConfigValue::ValueVariant* resolve(const std::string& section,
                                              const std::string& key) const;

    std::size_t count() const;
    std::size_t count_for_source(ConfigOverrideSource source) const;
    void clear();

    const std::vector<ConfigOverride>& entries() const { return overrides_; }

private:
    std::vector<ConfigOverride> overrides_;
};

} // namespace liz
