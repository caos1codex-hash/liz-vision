#include "engine/project/ProjectStatistics.h"

#include <iomanip>
#include <sstream>

namespace liz {

std::string ProjectStatistics::to_string() const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1);
    oss << "ProjectStats{"
        << " created=" << projects_created
        << " open=" << projects_open
        << " saved=" << projects_saved
        << " closed=" << projects_closed
        << " active=" << active_project
        << " assets=" << assets
        << " pipelines=" << pipelines
        << " services=" << services
        << " runtime=" << runtime_seconds << "s"
        << " }";
    return oss.str();
}

} // namespace liz
