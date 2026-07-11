#pragma once

#include "engine/config/ConfigSchema.h"
#include "engine/config/ConfigOverride.h"
#include "engine/config/ConfigEnvironment.h"
#include "engine/config/ConfigurationProfile.h"
#include "engine/config/ConfigurationStatistics.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace liz {

class ConfigurationManager; // Foundation (Sprint 22) — owned by EngineAPI, not here.
class EventBus;
class Configuration;

/// Advanced configuration layer built ON TOP of the Sprint 22 Foundation
/// ConfigurationManager. It adds schema validation, profiles, environment
/// overrides, runtime override priority, persistence, and dynamic reload.
///
/// This class composes (wraps) the Foundation manager — it never replaces it
/// and never duplicates its data; it keeps additional state (schema, override
/// stack, profile bindings) keyed by configuration UUID.
class AdvancedConfigurationManager {
public:
    AdvancedConfigurationManager();
    ~AdvancedConfigurationManager();

    // Non-copyable.
    AdvancedConfigurationManager(const AdvancedConfigurationManager&) = delete;
    AdvancedConfigurationManager& operator=(const AdvancedConfigurationManager&) = delete;

    // ── Lifecycle ──────────────────────────────────────────────────────

    /// Bind to the Foundation ConfigurationManager (owned by EngineAPI).
    void bind(ConfigurationManager* foundation);

    void initialize();
    void shutdown();

    /// Connect to the EventBus (same bus the Foundation uses).
    void set_event_bus(EventBus* bus);

    // ── Schema ─────────────────────────────────────────────────────────

    /// Access the shared schema (applies to every configuration).
    ConfigSchema& schema();
    const ConfigSchema& schema() const;

    /// Validate the active configuration against the schema.
    /// Increments stats counters and publishes ConfigSchemaValidated or
    /// ConfigValidationFailed.
    ConfigValidationResult validate_active();

    /// Validate an arbitrary configuration by UUID.
    ConfigValidationResult validate(const std::string& uuid);

    // ── Overrides ───────────────────────────────────────────────────────

    /// Apply a runtime override (highest priority) to (section,key).
    /// Increments override_count, publishes ConfigOverrideApplied, and
    /// writes the resolved value into the active configuration's section.
    bool apply_runtime_override(const std::string& section, const std::string& key,
                                ConfigValue::ValueVariant value);

    const ConfigOverrideStack& overrides() const;

    /// Collect environment overrides into the stack (source = Environment).
    std::size_t load_environment_overrides();

    // ── Profiles ───────────────────────────────────────────────────────

    /// Create a configuration tagged with a profile, attached to a file path.
    /// Delegates creation to the Foundation manager, then records the binding.
    Configuration* create_profiled(const std::string& name,
                                   ConfigurationProfile profile,
                                   const std::string& path = "");

    /// Bind a profile + path to an existing configuration UUID.
    bool bind_profile(const std::string& uuid, ConfigurationProfile profile,
                     const std::string& path = "");

    /// Activate the configuration bound to the given profile.
    /// Switches the Foundation active config and publishes ConfigProfileActivated.
    bool activate_profile(ConfigurationProfile profile);

    /// Currently active profile (Development/Production/Benchmark/Custom).
    ConfigurationProfile active_profile() const;

    // ── Persistence & Reload ────────────────────────────────────────────

    /// Save the configuration (by UUID) to its bound file path.
    bool save(const std::string& uuid);

    /// Reload the configuration (by UUID) from its bound file path and
    /// re-apply the override stack. Publishes ConfigReloaded.
    bool reload(const std::string& uuid);

    /// Reload the active configuration.
    bool reload_active();

    /// Serialize a configuration to text (in-memory, no file needed).
    std::string serialize(const std::string& uuid);

    // ── Statistics ──────────────────────────────────────────────────────

    ConfigurationStatistics statistics() const;
    void clear_statistics();

private:
    struct ProfileBinding {
        ConfigurationProfile profile = ConfigurationProfile::Custom;
        std::string         path;
    };

    void publish_event(int event_code, const std::string& message);
    void maybe_apply_overrides_to(Configuration& config);
    void set_value_on_config(Configuration& config,
                             const std::string& section, const std::string& key,
                             const ConfigValue::ValueVariant& value);

    EventBus*           event_bus_   = nullptr;
    ConfigurationManager* foundation_ = nullptr;

    ConfigSchema          schema_;
    ConfigOverrideStack   overrides_;
    ConfigEnvironment    environment_;

    std::unordered_map<std::string, ProfileBinding> bindings_; // uuid -> {profile, path}

    // Advanced counters.
    std::size_t schema_validations_  = 0;
    std::size_t reload_count_        = 0;
    std::size_t override_count_      = 0;
    std::size_t profile_switches_    = 0;
    std::size_t validation_failures_ = 0;
};

} // namespace liz
