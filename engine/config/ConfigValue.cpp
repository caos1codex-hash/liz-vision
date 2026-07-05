#include "engine/config/ConfigValue.h"

#include <sstream>
#include <iomanip>

namespace liz {

ConfigValue::ConfigValue(std::string key, ConfigValueType type, ValueVariant value,
                         ValueVariant default_value, std::string description)
    : key_(std::move(key))
    , type_(type)
    , value_(std::move(value))
    , default_value_(std::move(default_value))
    , description_(std::move(description))
{
    modified_ = (value_ != default_value_);
}

// ── Accessors ────────────────────────────────────────────────────────────

const std::string& ConfigValue::key() const             { return key_; }
ConfigValueType     ConfigValue::type() const           { return type_; }
const ConfigValue::ValueVariant& ConfigValue::value() const       { return value_; }
const ConfigValue::ValueVariant& ConfigValue::default_value() const { return default_value_; }
const std::string& ConfigValue::description() const     { return description_; }
bool ConfigValue::is_modified() const                   { return modified_; }

// ── Mutators ────────────────────────────────────────────────────────────

void ConfigValue::set_value(const ValueVariant& val) {
    value_ = val;
    modified_ = (value_ != default_value_);
}

void ConfigValue::reset() {
    value_ = default_value_;
    modified_ = false;
}

// ── Typed accessors ──────────────────────────────────────────────────

bool ConfigValue::as_bool() const {
    return std::get<bool>(value_);
}

int ConfigValue::as_int() const {
    return std::get<int>(value_);
}

double ConfigValue::as_double() const {
    return std::get<double>(value_);
}

const std::string& ConfigValue::as_string() const {
    return std::get<std::string>(value_);
}

// ── Utility ──────────────────────────────────────────────────────────

std::string ConfigValue::to_string() const {
    std::ostringstream oss;
    oss << "ConfigValue{key=\"" << key_
        << "\", type=" << config_value_type_to_string(type_);

    std::visit([&oss](const auto& v) {
        oss << ", value=";
        if constexpr (std::is_same_v<std::decay_t<decltype(v)>, bool>) {
            oss << (v ? "true" : "false");
        } else if constexpr (std::is_same_v<std::decay_t<decltype(v)>, std::string>) {
            oss << "\"" << v << "\"";
        } else {
            oss << std::fixed << std::setprecision(1) << v;
        }
    }, value_);

    if (modified_) oss << " [modified]";
    oss << "}";

    return oss.str();
}

} // namespace liz
