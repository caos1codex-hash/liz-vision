#pragma once

#include <string>

namespace liz {

/// Builder pattern for constructing an LIZ Engine instance.
///
/// Usage:
///   EngineBuilder builder;
///   builder.set_application_name("MyApp")
///           .set_application_version("1.0.0")
///           .enable_gpu(true)
///           .build();
///
/// build() constructs the configuration — it does NOT initialize
/// the engine.  Pass the built configuration to EngineAPI::initialize().
class EngineBuilder {
public:
    EngineBuilder();
    ~EngineBuilder() = default;

    // ── Application metadata ───────────────────────────────────────────

    /// Set the application name (shown in diagnostics).
    EngineBuilder& set_application_name(const std::string& name);

    /// Set the application version string.
    EngineBuilder& set_application_version(const std::string& version);

    // ── Feature flags ─────────────────────────────────────────────────

    /// Enable or disable GPU acceleration.
    EngineBuilder& enable_gpu(bool enabled);

    /// Enable or disable the plugin system.
    EngineBuilder& enable_plugins(bool enabled);

    /// Enable or disable diagnostics and profiling.
    EngineBuilder& enable_diagnostics(bool enabled);

    /// Enable or disable the asset management system.
    EngineBuilder& enable_assets(bool enabled);

    /// Enable or disable the event bus.
    EngineBuilder& enable_events(bool enabled);

    // ── Build ──────────────────────────────────────────────────────────

    /// Finalize the configuration.
    /// This does NOT start the engine — call EngineAPI::initialize() next.
    void build();

    // ── Accessors (read-only, after build) ──────────────────────────────

    const std::string& application_name() const;
    const std::string& application_version() const;
    bool gpu_enabled() const;
    bool plugins_enabled() const;
    bool diagnostics_enabled() const;
    bool assets_enabled() const;
    bool events_enabled() const;
    bool is_built() const;

private:
    std::string application_name_;
    std::string application_version_;
    bool gpu_enabled_         = false;
    bool plugins_enabled_     = true;
    bool diagnostics_enabled_ = true;
    bool assets_enabled_      = true;
    bool events_enabled_      = true;
    bool built_               = false;
};

} // namespace liz
