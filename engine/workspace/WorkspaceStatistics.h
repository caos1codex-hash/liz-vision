#pragma once

#include <cstddef>
#include <string>

namespace liz {

/// Cumulative statistics for the Workspace system.
struct WorkspaceStatistics {
    std::size_t workspaces_created = 0;
    std::size_t workspaces_open     = 0;
    std::size_t active_workspace   = 0;
    std::size_t projects_loaded    = 0;
    std::size_t assets_loaded      = 0;
    std::size_t pipelines_loaded   = 0;
    double     runtime_seconds     = 0.0;

    /// Generate a formatted summary string.
    std::string to_string() const;
};

} // namespace liz
