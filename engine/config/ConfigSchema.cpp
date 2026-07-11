#include "engine/config/ConfigSchema.h"
#include "engine/config/ConfigSection.h"
#include "engine/config/ConfigValue.h"
#include "engine/config/Configuration.h"

#include <sstream>
#include <regex>

namespace liz {

// ── ConfigValidationError ──────────────────────────────────────────────────

std::string ConfigValidationError::to_string() const {
    std::ostringstream oss;
    oss << "[" << section << "/" << key << "] " << reason;
    return oss.str();
}

std::string ConfigValidationResult::to_string() const {
    std::ostringstream oss;
    oss << "ConfigValidationResult{valid=" << (valid ? "true" : "false")
        << ", errors=" << errors.size();
    for (const auto& e : errors) {
        oss << ", " << e.to_string();
    }
    oss << "}";
    return oss.str();
}

// ── ConfigSchemaRule ──────────────────────────────────────────────────────

ConfigSchemaRule::ConfigSchemaRule(std::string key, ConfigValueType expected_type)
    : key_(std::move(key)), expected_type_(expected_type) {}

ConfigSchemaRule& ConfigSchemaRule::required(bool r) { required_ = r; return *this; }
ConfigSchemaRule& ConfigSchemaRule::with_default(std::string v) { default_string_ = std::move(v); return *this; }
ConfigSchemaRule& ConfigSchemaRule::min_value(double v) { has_min_ = true; min_value_ = v; return *this; }
ConfigSchemaRule& ConfigSchemaRule::max_value(double v) { has_max_ = true; max_value_ = v; return *this; }
ConfigSchemaRule& ConfigSchemaRule::regex_pattern(std::string pattern) { regex_pattern_ = std::move(pattern); return *this; }
ConfigSchemaRule& ConfigSchemaRule::allowed_values(std::vector<std::string> values) { allowed_values_ = std::move(values); return *this; }

const std::string&    ConfigSchemaRule::key() const { return key_; }
ConfigValueType       ConfigSchemaRule::expected_type() const { return expected_type_; }
bool                  ConfigSchemaRule::is_required() const { return required_; }
bool                  ConfigSchemaRule::has_min() const { return has_min_; }
bool                  ConfigSchemaRule::has_max() const { return has_max_; }
double                ConfigSchemaRule::min_value() const { return min_value_; }
double                ConfigSchemaRule::max_value() const { return max_value_; }
const std::string&    ConfigSchemaRule::regex_pattern() const { return regex_pattern_; }
const std::vector<std::string>& ConfigSchemaRule::allowed_values() const { return allowed_values_; }
const std::string&    ConfigSchemaRule::default_string() const { return default_string_; }

std::string ConfigSchemaRule::validate_value(const ConfigValue& value) const {
    // Type check.
    if (value.type() != expected_type_) {
        std::ostringstream oss;
        oss << "type mismatch: expected " << config_value_type_to_string(expected_type_)
            << ", got " << config_value_type_to_string(value.type());
        return oss.str();
    }

    // Numeric range checks (Int / Double).
    if (expected_type_ == ConfigValueType::Int || expected_type_ == ConfigValueType::Double) {
        double n = (expected_type_ == ConfigValueType::Int)
                       ? static_cast<double>(value.as_int())
                       : value.as_double();
        if (has_min_ && n < min_value_) {
            std::ostringstream oss;
            oss << "value " << n << " below minimum " << min_value_;
            return oss.str();
        }
        if (has_max_ && n > max_value_) {
            std::ostringstream oss;
            oss << "value " << n << " above maximum " << max_value_;
            return oss.str();
        }
    }

    // String constraints: allowed-values (enum set) takes precedence.
    if (expected_type_ == ConfigValueType::String) {
        const std::string& s = value.as_string();
        if (!allowed_values_.empty()) {
            bool ok = false;
            for (const auto& a : allowed_values_) {
                if (a == s) { ok = true; break; }
            }
            if (!ok) {
                std::ostringstream oss;
                oss << "value \"" << s << "\" not in allowed set";
                return oss.str();
            }
        }
        if (!regex_pattern_.empty()) {
            try {
                std::regex re(regex_pattern_);
                if (!std::regex_match(s, re)) {
                    std::ostringstream oss;
                    oss << "value \"" << s << "\" does not match pattern " << regex_pattern_;
                    return oss.str();
                }
            } catch (const std::regex_error&) {
                // Malformed schema pattern — treat as validation failure of the rule, not the value.
                return "invalid schema regex pattern: " + regex_pattern_;
            }
        }
    }

    return std::string();
}

// ── ConfigSchemaSection ───────────────────────────────────────────────────

ConfigSchemaSection::ConfigSchemaSection(std::string name) : name_(std::move(name)) {}

const std::string& ConfigSchemaSection::name() const { return name_; }

bool ConfigSchemaSection::add_rule(std::unique_ptr<ConfigSchemaRule> rule) {
    if (!rule) return false;
    const std::string& key = rule->key();
    if (rules_.count(key)) return false;
    rules_[key] = std::move(rule);
    return true;
}

const ConfigSchemaRule* ConfigSchemaSection::find(const std::string& key) const {
    auto it = rules_.find(key);
    return it == rules_.end() ? nullptr : it->second.get();
}

std::vector<std::string> ConfigSchemaSection::list() const {
    std::vector<std::string> keys;
    keys.reserve(rules_.size());
    for (const auto& [k, r] : rules_) keys.push_back(k);
    return keys;
}

std::size_t ConfigSchemaSection::count() const { return rules_.size(); }
void ConfigSchemaSection::clear() { rules_.clear(); }

// ── ConfigSchema ────────────────────────────────────────────────────────────

ConfigSchema::ConfigSchema() = default;

ConfigSchemaSection* ConfigSchema::create_section(const std::string& name) {
    if (sections_.count(name)) return nullptr;
    auto sec = std::make_unique<ConfigSchemaSection>(name);
    auto* ptr = sec.get();
    sections_[name] = std::move(sec);
    return ptr;
}

const ConfigSchemaSection* ConfigSchema::find_section(const std::string& name) const {
    auto it = sections_.find(name);
    return it == sections_.end() ? nullptr : it->second.get();
}

ConfigSchemaSection* ConfigSchema::find_section(const std::string& name) {
    auto it = sections_.find(name);
    return it == sections_.end() ? nullptr : it->second.get();
}

std::vector<std::string> ConfigSchema::list_sections() const {
    std::vector<std::string> names;
    names.reserve(sections_.size());
    for (const auto& [n, s] : sections_) names.push_back(n);
    return names;
}

std::size_t ConfigSchema::section_count() const { return sections_.size(); }
void ConfigSchema::clear() { sections_.clear(); }

ConfigValidationResult ConfigSchema::validate(const Configuration& config) const {
    ConfigValidationResult result;
    result.valid = true;

    // For each schema section, check that corresponding config section exists
    // and every rule passes (or required-missing fails).
    for (const auto& [sec_name, schema_sec] : sections_) {
        const ConfigSection* cfg_sec = config.find_section(sec_name);
        if (!cfg_sec) {
            for (const auto& rule_key : schema_sec->list()) {
                const auto* rule = schema_sec->find(rule_key);
                if (rule && rule->is_required()) {
                    result.errors.push_back({sec_name, rule_key, "required section missing"});
                    result.valid = false;
                }
            }
            continue;
        }

        for (const auto& rule_key : schema_sec->list()) {
            const auto* rule = schema_sec->find(rule_key);
            if (!rule) continue;

            const ConfigValue* val = cfg_sec->find(rule_key);
            if (!val) {
                if (rule->is_required()) {
                    result.errors.push_back({sec_name, rule_key, "required value missing"});
                    result.valid = false;
                }
                continue;
            }

            std::string reason = rule->validate_value(*val);
            if (!reason.empty()) {
                result.errors.push_back({sec_name, rule_key, reason});
                result.valid = false;
            }
        }
    }

    return result;
}

} // namespace liz
