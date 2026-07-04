#pragma once

#include <cstddef>
#include <string>

namespace liz {

/// Cumulative engine statistics gathered by the DiagnosticsManager.
///
/// These represent totals since the last reset, not point-in-time values.
/// Sourced from existing subsystems wherever possible.
struct EngineStatistics {
    // -- Timing --
    double   total_runtime_ms   = 0.0;

    // -- Processing --
    std::size_t frames_processed   = 0;

    // -- Events --
    std::size_t events_published   = 0;

    // -- Services --
    std::size_t services_registered = 0;

    // -- Resources --
    std::size_t resources_loaded   = 0;

    // -- Plugins --
    std::size_t plugins_active      = 0;

    // -- Batches --
    std::size_t batches_executed    = 0;

    // -- Inferences --
    std::size_t inferences_executed = 0;

    // -- GPU --
    std::size_t gpu_commands        = 0;

    // -- Assets (Sprint 14) --
    std::size_t assets_loaded      = 0;
    std::size_t assets_active       = 0;

    // -- Pipelines (Sprint 16) --
    std::size_t pipelines_active    = 0;
    std::size_t pipeline_nodes       = 0;
    double     pipeline_exec_time_ms = 0.0;
    std::size_t pipeline_errors      = 0;

    // -- Projects (Sprint 17) --
    std::size_t active_project       = 0;
    std::size_t open_projects        = 0;
    std::size_t saved_projects       = 0;

    // -- Workspaces (Sprint 18) --
    std::size_t active_workspace     = 0;
    std::size_t loaded_workspaces    = 0;
    std::size_t workspace_projects   = 0;

    /// Generate a formatted summary string.
    std::string to_string() const;
};

} // namespace liz
