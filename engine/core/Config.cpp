#include "engine/core/Config.h"

namespace liz {

// ── Singleton access ─────────────────────────────────────────────────────────
Config& Config::instance() {
    static Config config;
    return config;
}

// ── Core operations ──────────────────────────────────────────────────────────
void Config::set(std::string_view key, std::string_view value) {
    data_[std::string(key)] = std::string(value);
}

std::optional<std::string> Config::get(std::string_view key) const {
    auto it = data_.find(std::string(key));
    if (it != data_.end()) {
        return it->second;
    }
    return std::nullopt;
}

std::string Config::get(std::string_view key, std::string_view default_value) const {
    auto result = get(key);
    return result.has_value() ? result.value() : std::string(default_value);
}

bool Config::has(std::string_view key) const {
    return data_.find(std::string(key)) != data_.end();
}

bool Config::remove(std::string_view key) {
    auto it = data_.find(std::string(key));
    if (it != data_.end()) {
        data_.erase(it);
        return true;
    }
    return false;
}

std::size_t Config::size() const {
    return data_.size();
}

// ── Engine defaults ──────────────────────────────────────────────────────────
void Config::load_defaults() {
    data_.clear();

    // Engine core
    set("engine.name",           "LIZ Vision");
    set("engine.version",        "0.1.0");
    set("engine.threads",        "1");
    set("engine.log_level",      "INFO");

    // Scheduler
    set("scheduler.max_tasks",   "256");
    set("scheduler.mode",        "fifo");

    // Plugins
    set("plugins.enabled",       "true");
    set("plugins.directory",     "plugins/");

    // Video (placeholders for future sprints)
    set("video.input",           "");
    set("video.output",          "");
    set("video.fps",             "30");

    // GPU (placeholders for future sprints)
    set("gpu.device",            "0");
    set("gpu.enabled",           "false");

    // AI (placeholders for future sprints)
    set("ai.model_path",         "models/");
    set("ai.backend",            "cpu");
}

} // namespace liz