#pragma once

#include "engine/config/ConfigTypes.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace liz {

class Configuration;
class ConfigValue;

/// Validation error reported by ConfigSchema.
struct ConfigValidationError {
    std::string section;
    std::string key;
    std::string reason;

    std::string to_string() const;
};

/// Result of validating a Configuration against a schema.
struct ConfigValidationResult {
    bool        valid = true;
    std::vector<ConfigValidationError> errors;

    std::size_t error_count() const { return errors.size(); }
    std::string to_string() const;
};

/// A single validation rule for one config key (within a section).
class ConfigSchemaRule {
public:
    ConfigSchemaRule(std::string key, ConfigValueType expected_type);

    // ── Fluent rule configuration ────────────────────────────────────
    ConfigSchemaRule& required(bool r = true);
    ConfigSchemaRule& with_default(std::string v);
    ConfigSchemaRule& min_value(double v);    // numeric (Int/Double)
    ConfigSchemaRule& max_value(double v);    // numeric (Int/Double)
    ConfigSchemaRule& regex_pattern(std::string pattern); // String
    ConfigSchemaRule& allowed_values(std::vector<std::string> values); // enum-set

    // ── Accessors ───────────────────────────────────────────────────
    const std::string& key() const;
    ConfigValueType expected_type() const;
    bool is_required() const;
    bool has_min() const;
    bool has_max() const;
    double min_value() const;
    double max_value() const;
    const std::string& regex_pattern() const;
    const std::vector<std::string>& allowed_values() const;
    const std::string& default_string() const;

    /// Validate a single ConfigValue against this rule.
    /// Returns an error description (empty if valid).
    std::string validate_value(const ConfigValue& value) const;

private:
    std::string    key_;
    ConfigValueType expected_type_;
    bool           required_     = false;
    bool           has_min_     = false;
    bool           has_max_     = false;
    double         min_value_   = 0.0;
    double         max_value_   = 0.0;
    std::string    regex_pattern_;
    std::vector<std::string> allowed_values_;
    std::string    default_string_;
};

/// Schema for a configuration section: a set of named rules.
class ConfigSchemaSection {
public:
    explicit ConfigSchemaSection(std::string name);

    const std::string& name() const;

    /// Add a rule. Returns false if key already exists.
    bool add_rule(std::unique_ptr<ConfigSchemaRule> rule);

    const ConfigSchemaRule* find(const std::string& key) const;
    std::vector<std::string> list() const;
    std::size_t count() const;
    void clear();

private:
    std::string name_;
    std::unordered_map<std::string, std::unique_ptr<ConfigSchemaRule>> rules_;
};

/// Top-level schema: validates an entire Configuration.
class ConfigSchema {
public:
    ConfigSchema();

    /// Create or fetch a schema section. Returns nullptr if conflict.
    ConfigSchemaSection* create_section(const std::string& name);
    const ConfigSchemaSection* find_section(const std::string& name) const;
    ConfigSchemaSection* find_section(const std::string& name);
    std::vector<std::string> list_sections() const;
    std::size_t section_count() const;
    void clear();

    /// Validate a full Configuration against this schema.
    ConfigValidationResult validate(const Configuration& config) const;

private:
    std::unordered_map<std::string, std::unique_ptr<ConfigSchemaSection>> sections_;
};

} // namespace liz
