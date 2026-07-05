#include "engine/config/ConfigurationStatistics.h"

#include <sstream>

namespace liz {

std::string ConfigurationStatistics::to_string() const {
    std::ostringstream oss;
    oss << "ConfigurationStatistics{"
        << "created=" << created
        << ", destroyed=" << destroyed
        << ", active=" << active
        << ", sections=" << sections
        << ", values=" << values
        << ", modified=" << modified_values
        << "}";
    return oss.str();
}

} // namespace liz
