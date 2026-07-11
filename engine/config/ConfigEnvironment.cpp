#include "engine/config/ConfigEnvironment.h"

#include <cstdlib>
#include <sstream>
#include <string>

#if defined(_WIN32)
// MSVC's C runtime exposes the process environment table through _environ.
// Declared in the global scope, as required for an "extern C" linkage spec.
extern "C" char** _environ;
#endif

namespace liz {

namespace {

/// Uppercase ASCII in place.
std::string to_upper(std::string s) {
    for (char& c : s) {
        if (c >= 'a' && c <= 'z') c = static_cast<char>(c - ('a' - 'A'));
    }
    return s;
}

/// Build the env var name for a (section,key): <PREFIX>_<SECTION>_<KEY>,
/// with underscores replacing any whitespace in section/key.
std::string make_env_name(const std::string& prefix,
                          const std::string& section,
                          const std::string& key) {
    std::ostringstream oss;
    oss << prefix << "_" << to_upper(section) << "_" << to_upper(key);
    std::string name = oss.str();
    for (char& c : name) if (c == ' ') c = '_';
    return name;
}

} // namespace

ConfigEnvironment::ConfigEnvironment(std::string prefix) : prefix_(std::move(prefix)) {}

std::vector<ConfigOverride> ConfigEnvironment::read_overrides() const {
    // Walk the C runtime environ table, which exposes both names and values.
    std::vector<ConfigOverride> result;
    std::string prefix_upper = to_upper(prefix_) + "_";

#if defined(_WIN32)
    char** env = _environ;
#else
    extern char** environ;
    char** env = environ;
#endif
    if (!env) return result;

    for (char** var = env; *var; ++var) {
        std::string entry(*var);
        if (entry.rfind(prefix_upper, 0) != 0) continue; // must start with PREFIX_

        std::string rest = entry.substr(prefix_upper.size());
        // rest = "<SECTION>_<KEY>=<VALUE>"
        auto eq = rest.find('=');
        if (eq == std::string::npos) continue;
        std::string sk = rest.substr(0, eq);
        std::string value = rest.substr(eq + 1);

        // Split section / key on first underscore.
        auto us = sk.find('_');
        if (us == std::string::npos) continue;
        std::string section = sk.substr(0, us);
        std::string key = sk.substr(us + 1);
        if (key.empty()) continue;

        result.emplace_back(section, key, value, ConfigOverrideSource::Environment);
    }
    return result;
}

std::size_t ConfigEnvironment::populate(ConfigOverrideStack& stack) const {
    auto overrides = read_overrides();
    std::size_t added = overrides.size();
    for (auto& o : overrides) {
        stack.add(std::move(o));
    }
    return added;
}

std::string ConfigEnvironment::get(const std::string& section, const std::string& key) const {
    std::string name = make_env_name(prefix_, section, key);
    const char* v = std::getenv(name.c_str());
    return v ? std::string(v) : std::string();
}

} // namespace liz
