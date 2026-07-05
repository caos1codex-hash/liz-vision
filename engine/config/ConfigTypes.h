#pragma once

#include <string>

namespace liz {

/// Types of configuration values.
enum class ConfigValueType {
    Bool,
    Int,
    Double,
    String
};

/// Convert a ConfigValueType to a human-readable string.
inline const char* config_value_type_to_string(ConfigValueType type) {
    switch (type) {
        case ConfigValueType::Bool:   return "Bool";
        case ConfigValueType::Int:    return "Int";
        case ConfigValueType::Double: return "Double";
        case ConfigValueType::String: return "String";
    }
    return "Unknown";
}

} // namespace liz
