#pragma once

#include "engine/config/ConfigTypes.h"

#include <string>
#include <variant>

namespace liz {

/// Represents a single configurable value.
///
/// Supports bool, int, double, and string types.
/// Tracks key, type, current value, default value, description,
/// and a modified flag (true if value differs from default).
class ConfigValue {
public:
    /// The variant type that holds the actual value.
    using ValueVariant = std::variant<bool, int, double, std::string>;

    /// Construct with key, type, value, and optional default and description.
    ConfigValue(std::string key, ConfigValueType type, ValueVariant value,
                ValueVariant default_value = {}, std::string description = "");

    // Non-copyable, movable.
    ConfigValue(const ConfigValue&) = delete;
    ConfigValue& operator=(const ConfigValue&) = delete;
    ConfigValue(ConfigValue&&) noexcept = default;
    ConfigValue& operator=(ConfigValue&&) noexcept = default;
    ~ConfigValue() = default;

    // ── Accessors ────────────────────────────────────────────────────

    const std::string& key() const;
    ConfigValueType type() const;
    const ValueVariant& value() const;
    const ValueVariant& default_value() const;
    const std::string& description() const;
    bool is_modified() const;

    // ── Mutators ──────────────────────────────────────────────────────

    void set_value(const ValueVariant& val);
    void reset();

    // ── Typed accessors ──────────────────────────────────────────────

    bool as_bool() const;
    int as_int() const;
    double as_double() const;
    const std::string& as_string() const;

    // ── Utility ──────────────────────────────────────────────────────

    /// Generate a formatted summary string.
    std::string to_string() const;

private:
    std::string    key_;
    ConfigValueType type_;
    ValueVariant    value_;
    ValueVariant    default_value_;
    std::string    description_;
    bool            modified_ = false;
};

} // namespace liz
