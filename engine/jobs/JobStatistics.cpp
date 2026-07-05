#include "engine/jobs/JobStatistics.h"

#include <sstream>
#include <iomanip>

namespace liz {

std::string JobStatistics::to_string() const {
    std::ostringstream oss;
    oss << "JobStatistics{"
        << "created=" << jobs_created
        << ", running=" << jobs_running
        << ", completed=" << jobs_completed
        << ", failed=" << jobs_failed
        << ", cancelled=" << jobs_cancelled
        << ", total_exec_time=" << std::fixed << std::setprecision(1)
        << job_execution_time_ms << "ms"
        << "}";
    return oss.str();
}

} // namespace liz
