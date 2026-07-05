#pragma once

#include <cstdint>
#include <string>

namespace liz {

/// Category of a plugin.
enum class PluginCategory {
    AI,
    Video,
    GPU,
    Pipeline,
    IO,
    Cloud,
    UI,
    Utility,
    Unknown
};

/// Convert a PluginCategory to a human-readable string.
inline const char* plugin_category_to_string(PluginCategory cat) {
    switch (cat) {
        case PluginCategory::AI:      return "AI";
        case PluginCategory::Video:   return "Video";
        case PluginCategory::GPU:     return "GPU";
        case PluginCategory::Pipeline: return "Pipeline";
        case PluginCategory::IO:      return "IO";
        case PluginCategory::Cloud:   return "Cloud";
        case PluginCategory::UI:      return "UI";
        case PluginCategory::Utility: return "Utility";
        case PluginCategory::Unknown: return "Unknown";
    }
    return "Unknown";
}

/// Descriptor for a single plugin instance.
///
/// Contains metadata, state, and timing information.
/// Fully in-memory — no filesystem or DLL loading.
class PluginDescriptor {
public:
    PluginDescriptor();
    PluginDescriptor(const std::string& name, const std::string& author,
                     const std::string& version, PluginCategory category);

    // Non-copyable, movable.
    PluginDescriptor(const PluginDescriptor&) = delete;
    PluginDescriptor& operator=(const PluginDescriptor&) = delete;
    PluginDescriptor(PluginDescriptor&& other) noexcept;
    PluginDescriptor& operator=(PluginDescriptor&& other) noexcept;

    // ── Accessors ────────────────────────────────────────────────────────

    const std::string& uuid() const { return uuid_; }
    const std::string& name() const { return name_; }
    const std::string& author() const { return author_; }
    const std::string& version() const { return version_; }
    const std::string& description() const { return description_; }
    PluginCategory category() const { return category_; }
    const std::string& api_version() const { return api_version_; }
    const std::string& engine_version() const { return engine_version_; }

    bool is_enabled() const { return enabled_; }
    bool is_loaded() const { return loaded_; }

    double load_time_ms() const { return load_time_ms_; }
    double unload_time_ms() const { return unload_time_ms_; }

    // ── Mutators ─────────────────────────────────────────────────────────

    void set_enabled(bool enabled);
    void set_loaded(bool loaded);
    void set_load_time(double ms);
    void set_unload_time(double ms);
    void set_description(const std::string& desc);
    void set_api_version(const std::string& ver);
    void set_engine_version(const std::string& ver);

    /// Generate a formatted summary string.
    std::string to_string() const;

private:
    static std::string generate_uuid();

    std::string    uuid_;
    std::string    name_;
    std::string    author_;
    std::string    version_;
    std::string    description_;
    PluginCategory category_;
    std::string    api_version_;
    std::string    engine_version_;
    bool           enabled_;
    bool           loaded_;
    double         load_time_ms_;
    double         unload_time_ms_;
};

} // namespace liz
