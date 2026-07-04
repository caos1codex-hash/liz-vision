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

} // namespace liz
