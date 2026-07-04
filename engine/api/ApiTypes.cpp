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

} // namespace liz
