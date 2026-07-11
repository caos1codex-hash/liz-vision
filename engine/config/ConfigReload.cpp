#include "engine/config/ConfigReload.h"
#include "engine/config/AdvancedConfigurationManager.h"

namespace liz {

bool ConfigReloader::reload(AdvancedConfigurationManager& advanced, const std::string& uuid) {
    return advanced.reload(uuid);
}

bool ConfigReloader::reload_active(AdvancedConfigurationManager& advanced) {
    return advanced.reload_active();
}

} // namespace liz
