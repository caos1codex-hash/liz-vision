#include "engine/config/Configuration.h"

#include <algorithm>
#include <chrono>
#include <random>
#include <sstream>
#include <iomanip>

namespace liz {

Configuration::Configuration(std::string name, std::string description)
    : uuid_(generate_uuid())
    , name_(std::move(name))
    , description_(std::move(description))
{
    using namespace std::chrono;
    auto now = steady_clock::now();
    creation_time_ = static_cast<std::uint64_t>(
        duration_cast<milliseconds>(now.time_since_epoch()).count());
    modification_time_ = creation_time_;
}

// ── Accessors ────────────────────────────────────────────────────────────

const std::string& Configuration::uuid() const               { return uuid_; }
const std::string& Configuration::name() const               { return name_; }
const std::string& Configuration::description() const        { return description_; }
std::uint64_t Configuration::creation_time() const            { return creation_time_; }
std::uint64_t Configuration::modification_time() const        { return modification_time_; }

// ── Section management ────────────────────────────────────────────────

ConfigSection* Configuration::create_section(const std::string& name) {
    if (sections_.count(name)) return nullptr;

    auto section = std::make_unique<ConfigSection>(name);
    auto* ptr = section.get();
    sections_[name] = std::move(section);
    touch();
    return ptr;
}

bool Configuration::remove_section(const std::string& name) {
    auto it = sections_.find(name);
    if (it == sections_.end()) return false;
    sections_.erase(it);
    touch();
    return true;
}

ConfigSection* Configuration::find_section(const std::string& name) {
    auto it = sections_.find(name);
    if (it == sections_.end()) return nullptr;
    return it->second.get();
}

const ConfigSection* Configuration::find_section(const std::string& name) const {
    auto it = sections_.find(name);
    if (it == sections_.end()) return nullptr;
    return it->second.get();
}

std::vector<std::string> Configuration::list_sections() const {
    std::vector<std::string> names;
    names.reserve(sections_.size());
    for (const auto& [n, s] : sections_) {
        names.push_back(n);
    }
    std::sort(names.begin(), names.end());
    return names;
}

std::size_t Configuration::section_count() const {
    return sections_.size();
}

std::size_t Configuration::total_values() const {
    std::size_t total = 0;
    for (const auto& [n, s] : sections_) {
        total += s->count();
    }
    return total;
}

std::size_t Configuration::total_modified() const {
    std::size_t total = 0;
    for (const auto& [n, s] : sections_) {
        total += s->modified_count();
    }
    return total;
}

void Configuration::clear() {
    sections_.clear();
}

void Configuration::touch() {
    using namespace std::chrono;
    modification_time_ = static_cast<std::uint64_t>(
        duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count());
}

std::string Configuration::to_string() const {
    std::ostringstream oss;
    oss << "Configuration{uuid=" << uuid_
        << ", name=\"" << name_ << "\""
        << ", sections=" << section_count()
        << ", values=" << total_values()
        << ", modified=" << total_modified()
        << "}";
    return oss.str();
}

// ── UUID generation ────────────────────────────────────────────────────

std::string Configuration::generate_uuid() {
    static std::mt19937_64 rng(std::random_device{}());
    std::uint64_t value = rng();
    std::ostringstream oss;
    oss << std::hex << std::setw(8) << std::setfill('0') << (value & 0xFFFFFFFFu);
    return oss.str();
}

} // namespace liz
