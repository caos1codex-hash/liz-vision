#pragma once

#include <cstddef>
#include <string>

namespace liz {

/// Cumulative statistics for the Project system.
///
/// Tracks project lifecycle events across all managed projects.
struct ProjectStatistics {
    std::size_t projects_created = 0;
    std::size_t projects_open     = 0;
    std::size_t projects_saved   = 0;
    std::size_t projects_closed  = 0;
    std::size_t active_project   = 0;
    std::size_t assets           = 0;
    std::size_t pipelines        = 0;
    std::size_t services          = 0;
    double     runtime_seconds   = 0.0;

    /// Generate a formatted summary string.
    std::string to_string() const;
};

} // namespace liz
