#include "engine/pluginloader/PluginLoaderStatistics.h"

#include <iomanip>
#include <sstream>

namespace liz {

std::string PluginLoaderStatistics::to_string() const {
    std::ostringstream oss;
    oss << "PluginStats{"
        << "registered=" << registered
        << ", loaded=" << loaded
        << ", failed=" << failed
        << ", reloaded=" << reloaded
        << ", disabled=" << disabled
        << ", load_time=" << std::fixed << std::setprecision(1) << total_load_time_ms << "ms"
        << "}";
    return oss.str();
}

} // namespace liz
