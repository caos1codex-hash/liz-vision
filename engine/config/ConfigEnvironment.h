#pragma once

#include "engine/config/ConfigOverride.h"

#include <string>
#include <vector>

namespace liz {

/// Reads environment variables with a configurable prefix into
/// ConfigOverride entries of source Environment.
///
/// Variable naming convention: <PREFIX>_<SECTION>_<KEY>
/// Example:  LIZ_GRAPHICS_VSYNC=true  ->  section="Graphics", key="VSync"
class ConfigEnvironment {
public:
    explicit ConfigEnvironment(std::string prefix = "LIZ");

    /// Read all matching variables from the process environment and produce
    /// overrides. Values are stored as strings (source = Environment).
    std::vector<ConfigOverride> read_overrides() const;

    /// Add the collected environment overrides into the given stack.
    /// Returns the number of overrides added.
    std::size_t populate(ConfigOverrideStack& stack) const;

    /// Resolve a single key directly from the environment.
    /// Returns empty string when not set.
    std::string get(const std::string& section, const std::string& key) const;

    const std::string& prefix() const { return prefix_; }

private:
    std::string prefix_;
};

} // namespace liz
