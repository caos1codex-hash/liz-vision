#include "engine/diagnostics/PerformanceSnapshot.h"
#include "engine/diagnostics/EngineStatistics.h"

#include <sstream>
#include <iomanip>

namespace liz {

// ── PerformanceSnapshot ───────────────────────────────────────────────────

std::string PerformanceSnapshot::to_string() const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1);
    oss << "Snapshot[" << label << "]"
        << " fps=" << fps
        << " frame_time=" << frame_time_ms << "ms"
        << " cpu=" << cpu_time_ms << "ms"
        << " gpu=" << gpu_time_ms << "ms"
        << " ram=" << (ram_used_bytes / 1024) << "KB"
        << " vram=" << (vram_used_bytes / (1024 * 1024)) << "MB"
        << " events=" << events_published
        << " services=" << services_active
        << " resources=" << resources_active
        << " tasks=" << tasks_pending;
    return oss.str();
}

// ── EngineStatistics ──────────────────────────────────────────────────────

std::string EngineStatistics::to_string() const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1);
    oss << "Statistics"
        << " runtime=" << total_runtime_ms << "ms"
        << " frames=" << frames_processed
        << " events=" << events_published
        << " services=" << services_registered
        << " resources=" << resources_loaded
        << " plugins=" << plugins_active
        << " batches=" << batches_executed
        << " inferences=" << inferences_executed
        << " gpu_commands=" << gpu_commands
        << " assets=" << assets_loaded
        << " assets_active=" << assets_active
        << " pipelines=" << pipelines_active
        << " pipeline_nodes=" << pipeline_nodes
        << " active_project=" << active_project
        << " open_projects=" << open_projects
        << " saved_projects=" << saved_projects
        << " active_workspace=" << active_workspace
        << " loaded_workspaces=" << loaded_workspaces
        << " workspace_projects=" << workspace_projects
        << " cloud_pending=" << cloud_pending
        << " cloud_synced=" << cloud_synced
        << " cloud_conflicts=" << cloud_conflicts
        << " cloud_sync_time=" << cloud_sync_time_ms << "ms"
        << " plugins_loaded=" << plugins_loaded
        << " plugins_failed=" << plugins_failed
        << " plugins_reloaded=" << plugins_reloaded
        << " plugin_load_time=" << plugin_load_time_ms << "ms"
        << " jobs_created=" << jobs_created
        << " jobs_running=" << jobs_running
        << " jobs_completed=" << jobs_completed
        << " jobs_failed=" << jobs_failed
        << " job_exec_time=" << job_execution_time_ms << "ms";
    return oss.str();
}

} // namespace liz
