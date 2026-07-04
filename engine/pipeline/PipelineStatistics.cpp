#include "engine/pipeline/PipelineStatistics.h"

#include <sstream>
#include <iomanip>

namespace liz {

std::string PipelineStatistics::to_string() const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1);
    oss << "PipelineStats{"
        << "nodes=" << total_nodes
        << " edges=" << total_edges
        << " executed=" << nodes_executed
        << " failed=" << nodes_failed
        << " disabled=" << nodes_disabled
        << " time=" << execution_time_ms << "ms"
        << " valid=" << (validation_result ? "true" : "false")
        << "}";
    return oss.str();
}

} // namespace liz
