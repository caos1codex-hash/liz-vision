#include "engine/workspace/WorkspaceStatistics.h"

#include <iomanip>
#include <sstream>

namespace liz {

std::string WorkspaceStatistics::to_string() const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1);
    oss << "WorkspaceStats{"
        << " created=" << workspaces_created
        << " open=" << workspaces_open
        << " active=" << active_workspace
        << " projects=" << projects_loaded
        << " assets=" << assets_loaded
        << " pipelines=" << pipelines_loaded
        << " runtime=" << runtime_seconds << "s"
        << " }";
    return oss.str();
}

} // namespace liz
