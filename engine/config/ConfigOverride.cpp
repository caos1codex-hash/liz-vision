#include "engine/config/ConfigOverride.h"

#include <algorithm>
#include <sstream>

namespace liz {

// ── ConfigOverride ─────────────────────────────────────────────────────────

ConfigOverride::ConfigOverride(std::string section, std::string key,
                                ConfigValue::ValueVariant value,
                                ConfigOverrideSource source)
    : section_(std::move(section)), key_(std::move(key)),
      value_(std::move(value)), source_(source) {}

const std::string&           ConfigOverride::section() const { return section_; }
const std::string&           ConfigOverride::key() const { return key_; }
const ConfigValue::ValueVariant& ConfigOverride::value() const { return value_; }
ConfigOverrideSource         ConfigOverride::source() const { return source_; }
int                          ConfigOverride::priority() const { return config_override_source_priority(source_); }

std::string ConfigOverride::to_string() const {
    std::ostringstream oss;
    oss << "ConfigOverride{" << section_ << "/" << key_
        << ", source=" << config_override_source_to_string(source_)
        << ", priority=" << priority() << "}";
    return oss.str();
}

// ── ConfigOverrideStack ────────────────────────────────────────────────────

void ConfigOverrideStack::add(ConfigOverride override_entry) {
    // Replace any existing entry with same (section,key,source).
    for (auto& e : overrides_) {
        if (e.section() == override_entry.section() &&
            e.key() == override_entry.key() &&
            e.source() == override_entry.source()) {
            e = std::move(override_entry);
            return;
        }
    }
    overrides_.push_back(std::move(override_entry));
}

bool ConfigOverrideStack::remove(const std::string& section, const std::string& key,
                                 ConfigOverrideSource source) {
    auto it = std::remove_if(overrides_.begin(), overrides_.end(),
        [&](const ConfigOverride& e) {
            return e.section() == section && e.key() == key && e.source() == source;
        });
    if (it == overrides_.end()) return false;
    overrides_.erase(it, overrides_.end());
    return true;
}

const ConfigValue::ValueVariant* ConfigOverrideStack::resolve(const std::string& section,
                                                              const std::string& key) const {
    const ConfigOverride* best = nullptr;
    for (const auto& e : overrides_) {
        if (e.section() != section || e.key() != key) continue;
        if (!best || e.priority() > best->priority()) {
            best = &e;
        }
    }
    return best ? &best->value() : nullptr;
}

std::size_t ConfigOverrideStack::count() const { return overrides_.size(); }

std::size_t ConfigOverrideStack::count_for_source(ConfigOverrideSource source) const {
    std::size_t n = 0;
    for (const auto& e : overrides_) if (e.source() == source) ++n;
    return n;
}

void ConfigOverrideStack::clear() { overrides_.clear(); }

} // namespace liz
