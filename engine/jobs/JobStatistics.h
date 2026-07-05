#pragma once

#include <cstddef>
#include <string>

namespace liz {

/// Cumulative statistics for the Job System.
struct JobStatistics {
    std::size_t jobs_created    = 0;
    std::size_t jobs_running    = 0;
    std::size_t jobs_completed  = 0;
    std::size_t jobs_failed     = 0;
    std::size_t jobs_cancelled  = 0;
    double     job_execution_time_ms = 0.0;

    /// Generate a formatted summary string.
    std::string to_string() const;
};

} // namespace liz
