#include "demo/Sprint14Demo.h"

#include "engine/core/Engine.h"
#include "engine/core/Logger.h"
#include "engine/events/EventBus.h"
#include "engine/events/Event.h"
#include "engine/events/EventListener.h"
#include "engine/services/ServiceType.h"
#include "engine/services/ServiceRegistry.h"
#include "engine/diagnostics/DiagnosticsManager.h"
#include "engine/diagnostics/PerformanceSnapshot.h"
#include "engine/assets/AssetManager.h"
#include "engine/assets/AssetType.h"
#include "engine/assets/AssetDatabase.h"
#include "engine/assets/AssetHandle.h"

#include <iostream>
#include <memory>
#include <sstream>

// -- Asset Event Listener (Sprint 14) ---------------------------------------
class AssetEventListener : public liz::EventListener {
public:
    std::string_view name() const override { return "AssetEventListener"; }

    void on_event(const liz::Event& event) override {
        auto t = event.type();
        if (t == liz::EventType::AssetCreated ||
            t == liz::EventType::AssetLoaded ||
            t == liz::EventType::AssetUnloaded ||
            t == liz::EventType::AssetDestroyed) {
            std::ostringstream oss;
            oss << "  [AssetEventListener] " << event.info();
            LIZ_INFO(oss.str());
        }
    }

private:
    using liz::EventListener::on_event;
};

// ── Sprint14Demo implementation ───────────────────────────────────────────

int Sprint14Demo::run() {
    LIZ_INFO("--- Asset System Foundation (Sprint 14) ---");

    liz::EventBus bus;
    AssetEventListener listener;
    bus.subscribe(&listener);

    std::cout << std::endl;

    // 1. Create and initialize AssetManager.
    liz::AssetManager asset_mgr;
    asset_mgr.set_event_bus(&bus);
    asset_mgr.initialize();

    std::cout << std::endl;

    // 2. Register AssetManager in the Service Registry.
    liz::ServiceRegistry registry;
    registry.register_service("AssetManager", liz::ServiceType::AssetManager, "1.0.0");

    std::cout << std::endl;

    // 3. Create assets of various types.
    LIZ_INFO("Creating assets...");
    auto h_video   = asset_mgr.create_asset("cinematic_trailer.mp4",    liz::AssetType::Video,         50ULL * 1024ULL * 1024ULL, "1.0.0");
    auto h_image   = asset_mgr.create_asset("thumbnail_01.png",        liz::AssetType::Image,          2ULL * 1024ULL * 1024ULL, "1.0.0");
    auto h_audio   = asset_mgr.create_asset("background_music.mp3",    liz::AssetType::Audio,          5ULL * 1024ULL * 1024ULL, "1.0.0");
    auto h_model   = asset_mgr.create_asset("liz_upscaler_v2",         liz::AssetType::Model,         100ULL * 1024ULL * 1024ULL, "2.0.0");
    auto h_tensor  = asset_mgr.create_asset("weights_epoch50.bin",      liz::AssetType::Tensor,        200ULL * 1024ULL * 1024ULL, "1.0.0");
    auto h_shader  = asset_mgr.create_asset("bilinear_upscale.comp",     liz::AssetType::Shader,         32ULL * 1024ULL, "1.0.0");
    auto h_plugin  = asset_mgr.create_asset("denoise_plugin.so",        liz::AssetType::Plugin,        512ULL * 1024ULL, "1.0.0");
    auto h_config  = asset_mgr.create_asset("pipeline_config.json",     liz::AssetType::Configuration,  4096, "1.0.0");
    auto h_project = asset_mgr.create_asset("liz_studio_project.liz",   liz::AssetType::Project,       0, "1.0.0");
    auto h_temp    = asset_mgr.create_asset("temp_scratch_buffer",     liz::AssetType::Temporary,     64ULL * 1024ULL * 1024ULL, "0.0.0");

    std::cout << std::endl;

    // 4. Load some assets.
    LIZ_INFO("Loading assets...");
    asset_mgr.load(h_video.uuid());
    asset_mgr.load(h_model.uuid());
    asset_mgr.load(h_tensor.uuid());
    asset_mgr.load(h_shader.uuid());
    asset_mgr.load(h_config.uuid());

    std::cout << std::endl;

    // 5. Search for assets.
    LIZ_INFO("Searching assets...");
    {
        auto* found = asset_mgr.find_by_name("liz_upscaler_v2");
        if (found) {
            std::ostringstream oss;
            oss << "  find_by_name('liz_upscaler_v2'): " << found->info();
            LIZ_INFO(oss.str());
        }
    }
    {
        auto* missing = asset_mgr.find_by_name("nonexistent");
        std::ostringstream oss;
        oss << "  find_by_name('nonexistent'): " << (missing ? "found" : "nullptr");
        LIZ_INFO(oss.str());
    }
    {
        auto handle = asset_mgr.handle(h_video.uuid());
        std::ostringstream oss;
        oss << "  handle(video): " << handle.info();
        LIZ_INFO(oss.str());
    }

    std::cout << std::endl;

    // 6. Reload an asset.
    LIZ_INFO("Reloading asset...");
    asset_mgr.reload(h_model.uuid());

    std::cout << std::endl;

    // 7. Unload assets.
    LIZ_INFO("Unloading assets...");
    asset_mgr.unload(h_shader.uuid());
    asset_mgr.unload(h_temp.uuid());

    std::cout << std::endl;

    // 8. Destroy one asset.
    LIZ_INFO("Destroying asset...");
    asset_mgr.destroy_asset(h_temp.uuid());

    std::cout << std::endl;

    // 9. Statistics.
    LIZ_INFO("Asset Manager statistics:");
    asset_mgr.log_statistics();

    std::cout << std::endl;

    // 10. List all assets.
    LIZ_INFO("All managed assets:");
    auto names = asset_mgr.list();
    for (const auto& n : names) {
        auto* a = asset_mgr.find_by_name(n);
        if (a) {
            std::ostringstream oss;
            oss << "  " << a->info();
            LIZ_INFO(oss.str());
        }
    }

    std::cout << std::endl;

    // 11. Service Registry stats.
    LIZ_INFO("Service Registry:");
    registry.log_statistics();

    std::cout << std::endl;

    // 12. Diagnostics integration.
    LIZ_INFO("Diagnostics integration:");
    {
        liz::DiagnosticsDataProvider provider;
        auto stats = asset_mgr.statistics();
        provider.assets_loaded = stats.total_loaded;
        provider.assets_active = stats.active_assets;

        liz::DiagnosticsManager diag;
        diag.set_provider(&provider);

        auto engine_stats = diag.statistics();
        std::ostringstream oss;
        oss << "  " << engine_stats.to_string();
        LIZ_INFO(oss.str());
    }

    std::cout << std::endl;

    // 13. Cleanup.
    asset_mgr.clear();
    asset_mgr.shutdown();
    registry.clear();
    bus.unsubscribe(&listener);

    LIZ_INFO("Asset System demo completed");
    return 0;
}
