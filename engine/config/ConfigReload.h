#pragma once

#include <string>

namespace liz {

class AdvancedConfigurationManager;

/// Reloads a configuration from its source (file path) without restarting
/// the engine, then re-applies the current override stack. Publishes
/// ConfigReloaded via the AdvancedConfigurationManager's EventBus.
class ConfigReloader {
public:
    /// Reload a single configuration by its UUID from its bound file path.
    /// Returns true on success, false if the UUID is unknown or no path is bound.
    bool reload(AdvancedConfigurationManager& advanced, const std::string& uuid);

    /// Reload the currently active configuration. Returns true on success.
    bool reload_active(AdvancedConfigurationManager& advanced);
};

} // namespace liz
