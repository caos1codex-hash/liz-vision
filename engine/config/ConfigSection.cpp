#include "engine/config/ConfigSection.h"
#include "engine/core/Logger.h"

#include <algorithm>
#include <sstream>

namespace liz {

ConfigSection::ConfigSection(std::string name)
    : name_(std::move(name))
{}

const std::string& ConfigSection::name() const { return name_; }

// ── Value management ────────────────────────────────────────────────

bool ConfigSection::add_value(std::unique_ptr<ConfigValue> value) {
    if (!value) return false;
    const auto& key = value->key();
    if (values_.count(key)) {
        std::ostringstream oss;
        oss << "ConfigSection[" << name_ << "]: duplicate key \"" << key << "\"";
        LIZ_WARN(oss.str());
        return false;
    }
    values_[key] = std::move(value);
    return true;
}

bool ConfigSection::remove_value(const std::string& key) {
    return values_.erase(key) > 0;
}

const ConfigValue* ConfigSection::find(const std::string& key) const {
    auto it = values_.find(key);
    if (it == values_.end()) return nullptr;
    return it->second.get();
}

ConfigValue* ConfigSection::find_mutable(const std::string& key) {
    auto it = values_.find(key);
    if (it == values_.end()) return nullptr;
    return it->second.get();
}

bool ConfigSection::exists(const std::string& key) const {
    return values_.count(key) > 0;
}

std::vector<std::string> ConfigSection::list() const {
    std::vector<std::string> keys;
    keys.reserve(values_.size());
    for (const auto& [k, v] : values_) {
        keys.push_back(k);
    }
    std::sort(keys.begin(), keys.end());
    return keys;
}

std::size_t ConfigSection::count() const {
    return values_.size();
}

std::size_t ConfigSection::modified_count() const {
    std::size_t count = 0;
    for (const auto& [k, v] : values_) {
        if (v->is_modified()) ++count;
    }
    return count;
}

void ConfigSection::clear() {
    values_.clear();
}

} // namespace liz
