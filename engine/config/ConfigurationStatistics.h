#pragma once

#include <cstddef>
#include <string>

namespace liz {

/// Cumulative statistics for the Configuration System.
struct ConfigurationStatistics {
    // -- Foundation (Sprint 22) --
    std::size_t created          = 0;
    std::size_t destroyed        = 0;
    std::size_t active           = 0;
    std::size_t sections          = 0;
    std::size_t values            = 0;
    std::size_t modified_values   = 0;

    // -- Advanced (Sprint 23) --
    std::size_t schema_validations   = 0;
    std::size_t reload_count         = 0;
    std::size_t override_count       = 0;
    std::size_t profile_switches     = 0;
    std::size_t validation_failures  = 0;

    /// Generate a formatted summary string.
    std::string to_string() const;
};

} // namespace liz
