#pragma once

#include <cstddef>
#include <string>

namespace liz {

/// Statistics for a pipeline graph.
struct PipelineStatistics {
    std::size_t total_nodes      = 0;
    std::size_t total_edges      = 0;
    std::size_t nodes_executed   = 0;
    std::size_t nodes_failed     = 0;
    std::size_t nodes_disabled   = 0;
    double     execution_time_ms = 0.0;
    bool       validation_result = true;

    /// Generate a formatted summary string.
    std::string to_string() const;
};

} // namespace liz
