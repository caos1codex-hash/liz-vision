#pragma once

#include <cstddef>
#include <string>

namespace liz {

/// Cumulative statistics for the Plugin Loader system.
struct PluginLoaderStatistics {
    std::size_t registered  = 0;
    std::size_t loaded      = 0;
    std::size_t failed      = 0;
    std::size_t reloaded    = 0;
    std::size_t disabled    = 0;
    double     total_load_time_ms = 0.0;

    /// Generate a formatted summary string.
    std::string to_string() const;
};

} // namespace liz
