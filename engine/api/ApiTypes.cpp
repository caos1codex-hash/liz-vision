#include "engine/api/ApiTypes.h"

#include <sstream>
#include <iomanip>

namespace liz {

std::string ApiStatistics::to_string() const {
    std::ostringstream oss;
    oss << "ApiStatistics{"
        << "uptime=" << std::fixed << std::setprecision(1) << uptime_ms << "ms"
        << ", services=" << services_registered
        << ", assets=" << assets_active
        << ", frames=" << frames_processed
        << ", events=" << events_published
        << ", ram=" << (ram_used / 1024) << "KB"
        << ", vram=" << (vram_used / (1024 * 1024)) << "MB"
        << "}";
    return oss.str();
}

std::string ApiProjectStatistics::to_string() const {
    std::ostringstream oss;
    oss << "ProjectStatistics{"
        << "created=" << projects_created
        << ", open=" << projects_open
        << ", saved=" << projects_saved
        << ", closed=" << projects_closed
        << ", active=" << active_project
        << ", assets=" << assets
        << ", pipelines=" << pipelines
        << ", services=" << services
        << ", runtime=" << std::fixed << std::setprecision(1) << runtime_seconds << "s"
        << "}";
    return oss.str();
}

std::string ApiWorkspaceStatistics::to_string() const {
    std::ostringstream oss;
    oss << "WorkspaceStatistics{"
        << "created=" << workspaces_created
        << ", open=" << workspaces_open
        << ", active=" << active_workspace
        << ", projects=" << projects_loaded
        << ", assets=" << assets_loaded
        << ", pipelines=" << pipelines_loaded
        << ", runtime=" << std::fixed << std::setprecision(1) << runtime_seconds << "s"
        << "}";
    return oss.str();
}

std::string ApiCloudStatistics::to_string() const {
    std::ostringstream oss;
    oss << "CloudStatistics{"
        << "pending=" << items_pending
        << ", synced=" << items_synced
        << ", conflict=" << items_conflict
        << ", failed=" << items_failed
        << ", sync_time=" << std::fixed << std::setprecision(1) << total_sync_time_ms << "ms"
        << "}";
    return oss.str();
}

std::string ApiPluginStatistics::to_string() const {
    std::ostringstream oss;
    oss << "PluginStatistics{"
        << "registered=" << plugins_registered
        << ", loaded=" << plugins_loaded
        << ", failed=" << plugins_failed
        << ", reloaded=" << plugins_reloaded
        << ", load_time=" << std::fixed << std::setprecision(1) << total_load_time_ms << "ms"
        << "}";
    return oss.str();
}

std::string ApiJobStatistics::to_string() const {
    std::ostringstream oss;
    oss << "JobStatistics{"
        << "created=" << jobs_created
        << ", running=" << jobs_running
        << ", completed=" << jobs_completed
        << ", failed=" << jobs_failed
        << ", total_exec_time=" << std::fixed << std::setprecision(1) << total_execution_time_ms << "ms"
        << "}";
    return oss.str();
}

std::string ApiConfigurationStatistics::to_string() const {
    std::ostringstream oss;
    oss << "ConfigurationStatistics{"
        << "configs=" << configurations
        << ", active=" << active
        << ", sections=" << sections
        << ", values=" << values
        << ", modified=" << modified
        << "}";
    return oss.str();
}

} // namespace liz
