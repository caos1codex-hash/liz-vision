#include "engine/config/ConfigPersistence.h"
#include "engine/config/Configuration.h"
#include "engine/config/ConfigSection.h"
#include "engine/config/ConfigValue.h"
#include "engine/config/ConfigTypes.h"

#include <fstream>
#include <sstream>
#include <string>

namespace liz {

namespace {

/// Render a ConfigValue's current value as a string token (for the text format).
std::string render_value(const ConfigValue& val) {
    std::ostringstream oss;
    std::visit([&oss](const auto& v) {
        using T = std::decay_t<decltype(v)>;
        if constexpr (std::is_same_v<T, bool>) {
            oss << (v ? "true" : "false");
        } else if constexpr (std::is_same_v<T, std::string>) {
            oss << v;
        } else {
            oss << v;
        }
    }, val.value());
    return oss.str();
}

/// Render a default value (variant) for a given type.
std::string render_default(const ConfigValue& val) {
    std::ostringstream oss;
    std::visit([&oss](const auto& v) {
        using T = std::decay_t<decltype(v)>;
        if constexpr (std::is_same_v<T, bool>) {
            oss << (v ? "true" : "false");
        } else if constexpr (std::is_same_v<T, std::string>) {
            oss << v;
        } else {
            oss << v;
        }
    }, val.default_value());
    return oss.str();
}

/// Trim leading/trailing whitespace.
std::string trim(const std::string& s) {
    std::size_t a = s.find_first_not_of(" \t");
    if (a == std::string::npos) return "";
    std::size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

/// Parse a textual value into a ValueVariant of the given type.
ConfigValue::ValueVariant parse_typed(ConfigValueType type, const std::string& raw) {
    switch (type) {
        case ConfigValueType::Bool:
            return ConfigValue::ValueVariant(raw == "true" || raw == "1");
        case ConfigValueType::Int:
            return ConfigValue::ValueVariant(std::stoi(raw));
        case ConfigValueType::Double:
            return ConfigValue::ValueVariant(std::stod(raw));
        case ConfigValueType::String:
        default:
            return ConfigValue::ValueVariant(raw);
    }
}

} // namespace

std::string ConfigPersistence::serialize(const Configuration& config) {
    std::ostringstream oss;
    oss << "# LIZ Vision configuration: " << config.name() << "\n";
    if (!config.description().empty()) {
        oss << "# " << config.description() << "\n";
    }
    oss << "\n";

    for (const auto& sec_name : config.list_sections()) {
        const auto* sec = config.find_section(sec_name);
        if (!sec) continue;
        oss << "[" << sec_name << "]\n";
        for (const auto& key : sec->list()) {
            const auto* val = sec->find(key);
            if (!val) continue;
            // key = type:value  # description (default: <default>)
            oss << key << " = " << render_value(*val)
                << "  # type=" << config_value_type_to_string(val->type());
            if (!val->default_value().valueless_by_exception()) {
                // Only render default when it differs (a default was provided).
                // We compare a cheap string form to decide.
                std::string d = render_default(*val);
                if (!d.empty()) oss << ", default=" << d;
            }
            if (!val->description().empty()) oss << " — " << val->description();
            oss << "\n";
        }
        oss << "\n";
    }
    return oss.str();
}

bool ConfigPersistence::deserialize(const std::string& text, Configuration& config) {
    std::istringstream iss(text);
    std::string line;
    std::string current_section;
    bool ok = true;

    while (std::getline(iss, line)) {
        std::string t = trim(line);
        if (t.empty() || t[0] == '#') continue; // skip comments/blanks

        if (t.front() == '[' && t.back() == ']') {
            current_section = t.substr(1, t.size() - 2);
            if (!config.find_section(current_section)) {
                config.create_section(current_section);
            }
            continue;
        }

        auto eq = t.find('=');
        if (eq == std::string::npos) { ok = false; continue; }
        std::string key = trim(t.substr(0, eq));
        std::string rhs = trim(t.substr(eq + 1));

        // Optional inline comment: "value  # ..."
        std::string value_part = rhs;
        auto hash = rhs.find('#');
        if (hash != std::string::npos) {
            value_part = trim(rhs.substr(0, hash));
        }

        if (current_section.empty()) { ok = false; continue; }
        auto* sec = config.find_section(current_section);
        if (!sec) { config.create_section(current_section); sec = config.find_section(current_section); }

        // If the key already exists, infer type from the existing ConfigValue and update.
        ConfigValue* existing = sec->find_mutable(key);
        if (existing) {
            existing->set_value(parse_typed(existing->type(), value_part));
        } else {
            // New key: assume String unless it parses as bool/int/double.
            // Try bool, then int, then double, else string.
            if (value_part == "true" || value_part == "false") {
                sec->add_value(std::make_unique<ConfigValue>(
                    key, ConfigValueType::Bool,
                    ConfigValue::ValueVariant(value_part == "true")));
            } else {
                bool int_ok = true;
                try { std::stoi(value_part); } catch (...) { int_ok = false; }
                if (int_ok && !value_part.empty()) {
                    sec->add_value(std::make_unique<ConfigValue>(
                        key, ConfigValueType::Int,
                        ConfigValue::ValueVariant(std::stoi(value_part))));
                } else {
                    bool dbl_ok = true;
                    try { std::stod(value_part); } catch (...) { dbl_ok = false; }
                    if (dbl_ok && !value_part.empty()) {
                        sec->add_value(std::make_unique<ConfigValue>(
                            key, ConfigValueType::Double,
                            ConfigValue::ValueVariant(std::stod(value_part))));
                    } else {
                        sec->add_value(std::make_unique<ConfigValue>(
                            key, ConfigValueType::String,
                            ConfigValue::ValueVariant(value_part)));
                    }
                }
            }
        }
    }
    config.touch();
    return ok;
}

bool ConfigPersistence::save_to(const std::string& path, const Configuration& config) {
    std::ofstream out(path, std::ios::out | std::ios::trunc);
    if (!out.is_open()) return false;
    out << serialize(config);
    out.close();
    return out.good();
}

bool ConfigPersistence::load_from(const std::string& path, Configuration& config) {
    std::ifstream in(path);
    if (!in.is_open()) return false;
    std::ostringstream oss;
    oss << in.rdbuf();
    return deserialize(oss.str(), config);
}

} // namespace liz
