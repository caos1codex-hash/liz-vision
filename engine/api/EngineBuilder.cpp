#include "engine/api/EngineBuilder.h"
#include "engine/core/Logger.h"

#include <sstream>

namespace liz {

// ── Constructor ───────────────────────────────────────────────────────

EngineBuilder::EngineBuilder() = default;

// ── Application metadata ──────────────────────────────────────────────

EngineBuilder& EngineBuilder::set_application_name(const std::string& name) {
    application_name_ = name;
    return *this;
}

EngineBuilder& EngineBuilder::set_application_version(const std::string& version) {
    application_version_ = version;
    return *this;
}

// ── Feature flags ─────────────────────────────────────────────────────

EngineBuilder& EngineBuilder::enable_gpu(bool enabled) {
    gpu_enabled_ = enabled;
    return *this;
}

EngineBuilder& EngineBuilder::enable_plugins(bool enabled) {
    plugins_enabled_ = enabled;
    return *this;
}

EngineBuilder& EngineBuilder::enable_diagnostics(bool enabled) {
    diagnostics_enabled_ = enabled;
    return *this;
}

EngineBuilder& EngineBuilder::enable_assets(bool enabled) {
    assets_enabled_ = enabled;
    return *this;
}

EngineBuilder& EngineBuilder::enable_events(bool enabled) {
    events_enabled_ = enabled;
    return *this;
}

// ── Build ─────────────────────────────────────────────────────────────

void EngineBuilder::build() {
    built_ = true;

    std::ostringstream oss;
    oss << "EngineBuilder: configuration built — app='"
        << application_name_ << "' v'" << application_version_
        << "' gpu=" << (gpu_enabled_ ? "on" : "off")
        << " plugins=" << (plugins_enabled_ ? "on" : "off")
        << " diagnostics=" << (diagnostics_enabled_ ? "on" : "off")
        << " assets=" << (assets_enabled_ ? "on" : "off")
        << " events=" << (events_enabled_ ? "on" : "off");
    LIZ_INFO(oss.str());
}

// ── Accessors ──────────────────────────────────────────────────────────

const std::string& EngineBuilder::application_name() const { return application_name_; }
const std::string& EngineBuilder::application_version() const { return application_version_; }
bool EngineBuilder::gpu_enabled() const { return gpu_enabled_; }
bool EngineBuilder::plugins_enabled() const { return plugins_enabled_; }
bool EngineBuilder::diagnostics_enabled() const { return diagnostics_enabled_; }
bool EngineBuilder::assets_enabled() const { return assets_enabled_; }
bool EngineBuilder::events_enabled() const { return events_enabled_; }
bool EngineBuilder::is_built() const { return built_; }

} // namespace liz
