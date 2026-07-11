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

    // -- Cloud (Sprint 19) --
    std::size_t cloud_pending     = 0;
    std::size_t cloud_synced      = 0;
    std::size_t cloud_conflicts   = 0;
    double     cloud_sync_time_ms = 0.0;

    // -- Plugin Loader (Sprint 20) --
    std::size_t plugins_loaded        = 0;
    std::size_t plugins_failed        = 0;
    std::size_t plugins_reloaded      = 0;
    double     plugin_load_time_ms    = 0.0;

    // -- Job System (Sprint 21) --
    std::size_t jobs_created          = 0;
    std::size_t jobs_running          = 0;
    std::size_t jobs_completed        = 0;
    std::size_t jobs_failed           = 0;
    double     job_execution_time_ms  = 0.0;

    // -- Configuration System (Sprint 22) --
    std::size_t configurations        = 0;
    std::size_t configuration_sections = 0;
    std::size_t configuration_values  = 0;
    std::size_t modified_configuration_values = 0;

    // -- Advanced Configuration (Sprint 23) --
    std::size_t config_schema_validations  = 0;
    std::size_t config_reload_count        = 0;
    std::size_t config_override_count      = 0;
    std::size_t config_profile_switches    = 0;
    std::size_t config_validation_failures = 0;

    /// Generate a formatted summary string.
    std::string to_string() const;
};

} // namespace liz
