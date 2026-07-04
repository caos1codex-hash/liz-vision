#pragma once

#include <string>
#include <string_view>

namespace liz {

/// Abstract interface that every LIZ Vision plugin must implement.
///
/// A plugin is a named, versioned module that can be registered with
/// the PluginManager.  In future sprints plugins will expose processing
/// capabilities (upscaling, interpolation, detection, etc.).
///
/// Lifecycle:
///   on_register()  → called once when the plugin is added to the manager
///   on_unregister() → called once when the plugin is removed
class PluginInterface {
public:
    virtual ~PluginInterface() = default;

    // ── Identity ──────────────────────────────────────────────────────────────
    /// Human-readable plugin name (e.g. "Dummy Upscaler").
    virtual std::string_view name() const = 0;

    /// Plugin version string (e.g. "0.1.0").
    virtual std::string_view version() const = 0;

    /// Plugin category (e.g. "upscaler", "interpolation", "detection").
    virtual std::string_view category() const = 0;

    // ── Lifecycle hooks ───────────────────────────────────────────────────────
    /// Called when the plugin is registered with the PluginManager.
    virtual void on_register() = 0;

    /// Called when the plugin is unregistered from the PluginManager.
    virtual void on_unregister() = 0;
};

} // namespace liz