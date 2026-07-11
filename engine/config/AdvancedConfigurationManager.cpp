#include "engine/config/AdvancedConfigurationManager.h"
#include "engine/config/ConfigurationManager.h"
#include "engine/config/Configuration.h"
#include "engine/config/ConfigSection.h"
#include "engine/config/ConfigValue.h"
#include "engine/config/ConfigPersistence.h"
#include "engine/events/EventBus.h"
#include "engine/events/Event.h"
#include "engine/events/EventType.h"
#include "engine/core/Logger.h"

#include <sstream>

namespace liz {

// Event codes (Advanced, Sprint 23). 0..4 are reserved for Foundation.
// 5 = ConfigSchemaValidated
// 6 = ConfigValidationFailed
// 7 = ConfigReloaded
// 8 = ConfigProfileActivated
// 9 = ConfigOverrideApplied
namespace {
constexpr int kSchemaValidated = 5;
constexpr int kValidationFailed = 6;
constexpr int kReloaded        = 7;
constexpr int kProfileActivated = 8;
constexpr int kOverrideApplied  = 9;
} // namespace

AdvancedConfigurationManager::AdvancedConfigurationManager() = default;
AdvancedConfigurationManager::~AdvancedConfigurationManager() {
    if (foundation_) {
        // We do NOT own or shutdown the Foundation manager; EngineAPI does.
        foundation_ = nullptr;
    }
}

void AdvancedConfigurationManager::bind(ConfigurationManager* foundation) {
    foundation_ = foundation;
    LIZ_INFO("AdvancedConfigurationManager: bound to Foundation ConfigurationManager");
}

void AdvancedConfigurationManager::initialize() {
    LIZ_INFO("AdvancedConfigurationManager: initialized");
}

void AdvancedConfigurationManager::shutdown() {
    overrides_.clear();
    bindings_.clear();
    event_bus_ = nullptr;
    foundation_ = nullptr;
    LIZ_INFO("AdvancedConfigurationManager: shutdown");
}

void AdvancedConfigurationManager::set_event_bus(EventBus* bus) {
    event_bus_ = bus;
    LIZ_INFO("AdvancedConfigurationManager: connected to EventBus");
}

// ── Schema ─────────────────────────────────────────────────────────────

ConfigSchema& AdvancedConfigurationManager::schema() { return schema_; }
const ConfigSchema& AdvancedConfigurationManager::schema() const { return schema_; }

ConfigValidationResult AdvancedConfigurationManager::validate_active() {
    ++schema_validations_;
    if (!foundation_) {
        ConfigValidationResult r; r.valid = false;
        r.errors.push_back({"", "", "Advanced manager is not bound to a Foundation manager"});
        publish_event(kValidationFailed, "schema validation failed: no bound manager");
        ++validation_failures_;
        return r;
    }
    auto* active = foundation_->active();
    if (!active) {
        ConfigValidationResult r; r.valid = false;
        r.errors.push_back({"", "", "no active configuration to validate"});
        publish_event(kValidationFailed, "schema validation failed: no active configuration");
        ++validation_failures_;
        return r;
    }
    auto result = schema_.validate(*active);
    if (result.valid) {
        publish_event(kSchemaValidated, "schema validation passed for active configuration");
    } else {
        publish_event(kValidationFailed, result.to_string());
        ++validation_failures_;
    }
    return result;
}

ConfigValidationResult AdvancedConfigurationManager::validate(const std::string& uuid) {
    ++schema_validations_;
    if (!foundation_) {
        ConfigValidationResult r; r.valid = false;
        r.errors.push_back({"", "", "not bound to a Foundation manager"});
        publish_event(kValidationFailed, "schema validation failed: not bound");
        ++validation_failures_;
        return r;
    }
    auto* cfg = foundation_->find(uuid);
    if (!cfg) {
        ConfigValidationResult r; r.valid = false;
        r.errors.push_back({"", "", "configuration UUID not found: " + uuid});
        publish_event(kValidationFailed, "schema validation failed: UUID not found");
        ++validation_failures_;
        return r;
    }
    auto result = schema_.validate(*cfg);
    if (result.valid) {
        publish_event(kSchemaValidated, "schema validation passed for " + uuid);
    } else {
        publish_event(kValidationFailed, result.to_string());
        ++validation_failures_;
    }
    return result;
}

// ── Overrides ───────────────────────────────────────────────────────────

bool AdvancedConfigurationManager::apply_runtime_override(const std::string& section,
                                                         const std::string& key,
                                                         ConfigValue::ValueVariant value) {
    overrides_.add(ConfigOverride(section, key, value, ConfigOverrideSource::Runtime));
    ++override_count_;

    if (foundation_) {
        if (auto* active = foundation_->active()) {
            maybe_apply_overrides_to(*active);
        }
    }

    std::ostringstream oss;
    oss << "override applied: " << section << "/" << key
        << " (runtime, priority=" << config_override_source_priority(ConfigOverrideSource::Runtime) << ")";
    publish_event(kOverrideApplied, oss.str());
    LIZ_INFO(std::string("AdvancedConfigurationManager: ") + oss.str());
    return true;
}

const ConfigOverrideStack& AdvancedConfigurationManager::overrides() const { return overrides_; }

std::size_t AdvancedConfigurationManager::load_environment_overrides() {
    std::size_t n = environment_.populate(overrides_);
    if (n > 0) {
        std::ostringstream oss;
        oss << "loaded " << n << " environment overrides (prefix=" << environment_.prefix() << ")";
        LIZ_INFO(oss.str());
        if (foundation_) {
            if (auto* active = foundation_->active()) {
                maybe_apply_overrides_to(*active);
            }
        }
    }
    return n;
}

// ── Profiles ─────────────────────────────────────────────────────────────

Configuration* AdvancedConfigurationManager::create_profiled(const std::string& name,
                                                              ConfigurationProfile profile,
                                                              const std::string& path) {
    if (!foundation_) return nullptr;
    Configuration* cfg = foundation_->create_configuration(name);
    if (!cfg) return nullptr;
    bind_profile(cfg->uuid(), profile, path);
    return cfg;
}

bool AdvancedConfigurationManager::bind_profile(const std::string& uuid,
                                                ConfigurationProfile profile,
                                                const std::string& path) {
    if (!foundation_ || !foundation_->find(uuid)) return false;
    bindings_[uuid] = ProfileBinding{profile, path};
    return true;
}

bool AdvancedConfigurationManager::activate_profile(ConfigurationProfile profile) {
    if (!foundation_) return false;
    for (const auto& [uuid, binding] : bindings_) {
        if (binding.profile == profile) {
            if (foundation_->set_active(uuid)) {
                ++profile_switches_;
                std::ostringstream oss;
                oss << "profile activated: " << configuration_profile_to_string(profile)
                    << " (uuid=" << uuid << ")";
                publish_event(kProfileActivated, oss.str());
                LIZ_INFO(std::string("AdvancedConfigurationManager: ") + oss.str());
                return true;
            }
        }
    }
    return false;
}

ConfigurationProfile AdvancedConfigurationManager::active_profile() const {
    if (!foundation_) return ConfigurationProfile::Custom;
    auto* a = foundation_->active();
    if (!a) return ConfigurationProfile::Custom;
    auto it = bindings_.find(a->uuid());
    return (it == bindings_.end()) ? ConfigurationProfile::Custom : it->second.profile;
}

// ── Persistence & Reload ──────────────────────────────────────────────────

bool AdvancedConfigurationManager::save(const std::string& uuid) {
    if (!foundation_) return false;
    auto* cfg = foundation_->find(uuid);
    if (!cfg) return false;
    auto it = bindings_.find(uuid);
    if (it == bindings_.end() || it->second.path.empty()) return false;
    return ConfigPersistence::save_to(it->second.path, *cfg);
}

bool AdvancedConfigurationManager::reload(const std::string& uuid) {
    if (!foundation_) return false;
    auto* cfg = foundation_->find(uuid);
    if (!cfg) return false;
    auto it = bindings_.find(uuid);
    if (it == bindings_.end() || it->second.path.empty()) return false;
    if (!ConfigPersistence::load_from(it->second.path, *cfg)) return false;

    ++reload_count_;
    maybe_apply_overrides_to(*cfg);

    std::ostringstream oss;
    oss << "configuration reloaded: " << cfg->name() << " (from " << it->second.path << ")";
    publish_event(kReloaded, oss.str());
    LIZ_INFO(std::string("AdvancedConfigurationManager: ") + oss.str());
    return true;
}

bool AdvancedConfigurationManager::reload_active() {
    if (!foundation_) return false;
    auto* a = foundation_->active();
    return a ? reload(a->uuid()) : false;
}

std::string AdvancedConfigurationManager::serialize(const std::string& uuid) {
    if (!foundation_) return "";
    auto* cfg = foundation_->find(uuid);
    return cfg ? ConfigPersistence::serialize(*cfg) : "";
}

// ── Statistics ────────────────────────────────────────────────────────────

ConfigurationStatistics AdvancedConfigurationManager::statistics() const {
    ConfigurationStatistics s;
    if (!foundation_) return s;
    auto base = foundation_->statistics();
    s.created             = base.created;
    s.destroyed           = base.destroyed;
    s.active              = base.active;
    s.sections            = base.sections;
    s.values              = base.values;
    s.modified_values     = base.modified_values;
    s.schema_validations  = schema_validations_;
    s.reload_count        = reload_count_;
    s.override_count      = override_count_;
    s.profile_switches    = profile_switches_;
    s.validation_failures = validation_failures_;
    return s;
}

void AdvancedConfigurationManager::clear_statistics() {
    schema_validations_  = 0;
    reload_count_        = 0;
    override_count_      = 0;
    profile_switches_    = 0;
    validation_failures_ = 0;
}

// ── Event publishing ──────────────────────────────────────────────────────

void AdvancedConfigurationManager::publish_event(int event_code, const std::string& message) {
    if (!event_bus_) return;

    EventType type = EventType::Custom;
    switch (event_code) {
        case kSchemaValidated:  type = EventType::ConfigSchemaValidated; break;
        case kValidationFailed: type = EventType::ConfigValidationFailed; break;
        case kReloaded:         type = EventType::ConfigReloaded;         break;
        case kProfileActivated:type = EventType::ConfigProfileActivated; break;
        case kOverrideApplied:  type = EventType::ConfigOverrideApplied;   break;
        default: type = EventType::Custom; break;
    }
    Event event(type, "AdvancedConfigurationManager", message);
    event_bus_->publish(event);
}

// ── Override application ──────────────────────────────────────────────────

void AdvancedConfigurationManager::maybe_apply_overrides_to(Configuration& config) {
    for (const auto& ov : overrides_.entries()) {
        const auto* val = overrides_.resolve(ov.section(), ov.key());
        if (val) {
            set_value_on_config(config, ov.section(), ov.key(), *val);
        }
    }
}

void AdvancedConfigurationManager::set_value_on_config(Configuration& config,
                                                       const std::string& section,
                                                       const std::string& key,
                                                       const ConfigValue::ValueVariant& value) {
    auto* sec = config.find_section(section);
    if (!sec) return;
    ConfigValue* v = sec->find_mutable(key);
    if (!v) return;

    // Coerce the variant-like value into the stored ConfigValue type.
    // ConfigValue::ValueVariant is variant<bool,int,double,string>. We map
    // whatever source variant holds into the target's expected type.
    if (v->type() == ConfigValueType::Bool) {
        if (std::holds_alternative<bool>(value))            v->set_value(std::get<bool>(value));
        else if (std::holds_alternative<int>(value))        v->set_value(std::get<int>(value) != 0);
        else if (std::holds_alternative<std::string>(value))v->set_value(std::get<std::string>(value) == "true");
    } else if (v->type() == ConfigValueType::Int) {
        if (std::holds_alternative<int>(value))             v->set_value(std::get<int>(value));
        else if (std::holds_alternative<double>(value))     v->set_value(static_cast<int>(std::get<double>(value)));
        else if (std::holds_alternative<bool>(value))       v->set_value(std::get<bool>(value) ? 1 : 0);
        else if (std::holds_alternative<std::string>(value)){
            try { v->set_value(std::stoi(std::get<std::string>(value))); }
            catch (...) { /* ignore parse failure */ }
        }
    } else if (v->type() == ConfigValueType::Double) {
        if (std::holds_alternative<double>(value))          v->set_value(std::get<double>(value));
        else if (std::holds_alternative<int>(value))        v->set_value(static_cast<double>(std::get<int>(value)));
        else if (std::holds_alternative<std::string>(value)){
            try { v->set_value(std::stod(std::get<std::string>(value))); }
            catch (...) { /* ignore parse failure */ }
        }
    } else { // String
        if (std::holds_alternative<std::string>(value))     v->set_value(std::get<std::string>(value));
        else if (std::holds_alternative<bool>(value))       v->set_value(std::get<bool>(value) ? std::string("true") : std::string("false"));
        else { /* fallback: stringify via visitor below */
            std::ostringstream oss;
            std::visit([&oss](const auto& x) {
                using T = std::decay_t<decltype(x)>;
                if constexpr (std::is_same_v<T, bool>)        oss << (x ? "true" : "false");
                else if constexpr (std::is_same_v<T, std::string>) oss << x;
                else oss << x;
            }, value);
            v->set_value(oss.str());
        }
    }
    config.touch();
}

} // namespace liz
