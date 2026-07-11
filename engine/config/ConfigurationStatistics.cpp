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
        << ", schema_validations=" << schema_validations
        << ", reload_count=" << reload_count
        << ", override_count=" << override_count
        << ", profile_switches=" << profile_switches
        << ", validation_failures=" << validation_failures
        << "}";
    return oss.str();
}

} // namespace liz
