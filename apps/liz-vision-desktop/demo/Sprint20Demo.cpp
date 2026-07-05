#include "demo/Sprint20Demo.h"

#include "engine/core/Logger.h"
#include "engine/events/EventBus.h"
#include "engine/events/Event.h"
#include "engine/events/EventListener.h"
#include "engine/events/EventType.h"
#include "engine/services/ServiceRegistry.h"
#include "engine/services/ServiceType.h"
#include "engine/diagnostics/DiagnosticsManager.h"
#include "engine/diagnostics/EngineStatistics.h"
#include "engine/pluginloader/PluginLoader.h"
#include "engine/pluginloader/PluginManifest.h"
#include "engine/pluginloader/PluginDescriptor.h"
#include "engine/pluginloader/PluginLoaderStatistics.h"

#include <iostream>
#include <sstream>

// -- Plugin Event Listener ---------------------------------------------------
class PluginEventListener : public liz::EventListener {
public:
    std::string_view name() const override { return "PluginEventListener"; }

    void on_event(const liz::Event& event) override {
        auto t = event.type();
        if (t == liz::EventType::PluginLoading ||
            t == liz::EventType::PluginLoaded ||
            t == liz::EventType::PluginUnloading ||
            t == liz::EventType::PluginUnloaded ||
            t == liz::EventType::PluginReloaded ||
            t == liz::EventType::PluginLoadFailed) {
            std::ostringstream oss;
            oss << "  [PluginEventListener] " << event.info();
            LIZ_INFO(oss.str());
        }
    }

private:
    using liz::EventListener::on_event;
};

// -- Helper: create a manifest from parameters ─────────────────────────────
static liz::PluginManifest make_manifest(
    const std::string& name,
    const std::string& author,
    const std::string& version,
    const std::string& description,
    const std::string& engine_ver = "1.0.0",
    const std::string& api_ver = "1.0.0")
{
    liz::PluginManifest m;
    m.set_name(name);
    m.set_author(author);
    m.set_version(version);
    m.set_description(description);
    m.set_required_engine_version(engine_ver);
    m.set_api_version(api_ver);
    m.set_website("https://liz-vision.dev");
    m.set_license("MIT");
    return m;
}

// -- Demo implementation ─────────────────────────────────────────────────────
int Sprint20Demo::run() {
    LIZ_INFO("--- Plugin Loader Foundation (Sprint 20) ---");

    liz::EventBus bus;
    PluginEventListener listener;
    bus.subscribe(&listener);

    liz::ServiceRegistry registry;
    registry.register_service("PluginLoader", liz::ServiceType::PluginLoader, "1.0.0");

    std::cout << std::endl;

    // ==================================================================
    // 1. Create PluginLoader
    // ==================================================================
    LIZ_INFO("[1] Creating PluginLoader...");

    liz::PluginLoader loader;
    loader.initialize();
    loader.set_event_bus(&bus);

    LIZ_INFO("  PluginLoader initialized and connected to EventBus");

    std::cout << std::endl;

    // ==================================================================
    // 2. Create 5 plugin manifests and register them
    // ==================================================================
    LIZ_INFO("[2] Creating and registering 5 plugin manifests...");

    auto m1 = make_manifest("AI Upscaler",        "LIZ Team", "2.0.0", "Neural network-based video upscaling");
    auto m2 = make_manifest("Frame Interpolator", "LIZ Team", "1.5.0", "AI-powered frame interpolation");
    auto m3 = make_manifest("Video Decoder",      "LIZ Team", "3.1.0", "FFmpeg-based video decoding");
    auto m4 = make_manifest("Tensor Converter",   "LIZ Team", "1.0.0", "Convert video frames to tensors");
    auto m5 = make_manifest("Cloud Connector",     "LIZ Team", "0.9.0", "Cloud sync and remote storage");

    std::string uuid1 = loader.register_plugin(m1);
    std::string uuid2 = loader.register_plugin(m2);
    std::string uuid3 = loader.register_plugin(m3);
    std::string uuid4 = loader.register_plugin(m4);
    std::string uuid5 = loader.register_plugin(m5);

    {
        std::ostringstream oss;
        oss << "  Registered: " << loader.list_registered().size() << " plugins";
        LIZ_INFO(oss.str());
    }

    for (const auto& name : loader.list_registered()) {
        std::ostringstream oss;
        oss << "    - " << name;
        LIZ_INFO(oss.str());
    }

    std::cout << std::endl;

    // ==================================================================
    // 3. Load all plugins
    // ==================================================================
    LIZ_INFO("[3] Loading all plugins...");

    loader.load_all();

    {
        std::ostringstream oss;
        oss << "  Loaded plugins: " << loader.loaded_plugins().size();
        LIZ_INFO(oss.str());
    }

    for (const auto* desc : loader.loaded_plugins()) {
        std::ostringstream oss;
        oss << "    - " << desc->to_string();
        LIZ_INFO(oss.str());
    }

    std::cout << std::endl;

    // ==================================================================
    // 4. Reload one plugin
    // ==================================================================
    LIZ_INFO("[4] Reloading 'Frame Interpolator'...");

    bool reloaded = loader.reload_plugin(uuid2);
    {
        std::ostringstream oss;
        oss << "  Reload result: " << (reloaded ? "success" : "failed");
        LIZ_INFO(oss.str());
    }

    std::cout << std::endl;

    // ==================================================================
    // 5. Unload one plugin
    // ==================================================================
    LIZ_INFO("[5] Unloading 'Video Decoder'...");

    bool unloaded = loader.unload_plugin(uuid3);
    {
        std::ostringstream oss;
        oss << "  Unload result: " << (unloaded ? "success" : "failed");
        LIZ_INFO(oss.str());
    }

    std::cout << std::endl;

    // ==================================================================
    // 6. List all plugins and show states
    // ==================================================================
    LIZ_INFO("[6] Listing all plugins with states...");

    std::vector<std::pair<std::string, std::string>> uuids = {
        {uuid1, "AI Upscaler"},
        {uuid2, "Frame Interpolator"},
        {uuid3, "Video Decoder"},
        {uuid4, "Tensor Converter"},
        {uuid5, "Cloud Connector"}
    };
    for (const auto& [uuid, name] : uuids) {
        std::ostringstream oss;
        oss << "  " << name << " — loaded=" << (loader.is_loaded(uuid) ? "true" : "false");
        LIZ_INFO(oss.str());
    }

    std::cout << std::endl;

    // ==================================================================
    // 7. Show statistics
    // ==================================================================
    LIZ_INFO("[7] Plugin loader statistics:");
    {
        auto stats = loader.statistics();
        std::ostringstream oss;
        oss << "  " << stats.to_string();
        LIZ_INFO(oss.str());
    }

    std::cout << std::endl;

    // ==================================================================
    // 8. Show EventBus statistics
    // ==================================================================
    LIZ_INFO("[8] EventBus statistics:");
    {
        auto bus_stats = bus.statistics();
        std::ostringstream oss;
        oss << "  Total published: " << bus_stats.total_published
            << "  Total delivered: " << bus_stats.total_delivered
            << "  Listeners: " << bus_stats.listener_count;
        LIZ_INFO(oss.str());
    }

    std::cout << std::endl;

    // ==================================================================
    // 9. Diagnostics report with plugin data
    // ==================================================================
    LIZ_INFO("[9] Diagnostics report (updated with plugin metrics):");
    {
        liz::DiagnosticsDataProvider provider;
        provider.events_published   = bus.event_count();
        provider.services_registered = registry.count();

        auto pl_stats = loader.statistics();
        provider.plugins_loaded   = pl_stats.loaded;
        provider.plugins_failed   = pl_stats.failed;
        provider.plugins_reloaded = pl_stats.reloaded;
        provider.plugin_load_time_ms = pl_stats.total_load_time_ms;

        liz::DiagnosticsManager diag;
        diag.set_provider(&provider);
        diag.capture_snapshot("after_sprint20");

        auto engine_stats = diag.statistics();
        std::ostringstream oss;
        oss << "  " << engine_stats.to_string();
        LIZ_INFO(oss.str());

        diag.print_report();
    }

    std::cout << std::endl;

    // ==================================================================
    // Cleanup
    // ==================================================================
    loader.clear();
    loader.shutdown();
    registry.clear();
    bus.unsubscribe(&listener);

    LIZ_INFO("Plugin Loader demo completed");
    return 0;
}
