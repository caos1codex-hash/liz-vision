#include "demo/Sprint22Demo.h"

#include "engine/core/Logger.h"
#include "engine/events/EventBus.h"
#include "engine/events/Event.h"
#include "engine/events/EventListener.h"
#include "engine/events/EventType.h"
#include "engine/services/ServiceRegistry.h"
#include "engine/services/ServiceType.h"
#include "engine/diagnostics/DiagnosticsManager.h"
#include "engine/diagnostics/EngineStatistics.h"
#include "engine/config/ConfigurationManager.h"
#include "engine/config/Configuration.h"
#include "engine/config/ConfigSection.h"
#include "engine/config/ConfigValue.h"
#include "engine/config/ConfigTypes.h"
#include "engine/config/ConfigurationStatistics.h"
#include "engine/api/EngineAPI.h"
#include "engine/api/ApiTypes.h"
#include "engine/api/EngineBuilder.h"

#include <iostream>
#include <memory>
#include <sstream>
#include <vector>

// -- Config Event Listener ------------------------------------------------
class ConfigEventListener : public liz::EventListener {
public:
    std::string_view name() const override { return "ConfigEventListener"; }

    void on_event(const liz::Event& event) override {
        auto t = event.type();
        if (t == liz::EventType::ConfigurationCreated ||
            t == liz::EventType::ConfigurationDestroyed ||
            t == liz::EventType::ConfigurationActivated ||
            t == liz::EventType::ConfigurationValueChanged ||
            t == liz::EventType::ConfigurationReset) {
            std::ostringstream oss;
            oss << "  [ConfigEventListener] " << event.info();
            LIZ_INFO(oss.str());
        }
    }

private:
    using liz::EventListener::on_event;
};

// -- Helper: create a config value inline ──────────────────────────────
static std::unique_ptr<liz::ConfigValue> make_val(
    const std::string& key, liz::ConfigValueType type,
    liz::ConfigValue::ValueVariant value,
    liz::ConfigValue::ValueVariant default_val = {},
    const std::string& desc = "")
{
    return std::make_unique<liz::ConfigValue>(key, type, value, default_val, desc);
}

// -- Demo implementation ─────────────────────────────────────────────────
int Sprint22Demo::run() {
    LIZ_INFO("--- Configuration System Foundation (Sprint 22) ---");

    liz::EventBus bus;
    ConfigEventListener listener;
    bus.subscribe(&listener);

    liz::ServiceRegistry registry;
    registry.register_service("ConfigurationManager", liz::ServiceType::ConfigurationManager, "1.0.0");

    std::cout << std::endl;

    // ==================================================================
    // 1. Create ConfigurationManager
    // ==================================================================
    LIZ_INFO("[1] Creating ConfigurationManager...");

    liz::ConfigurationManager config_mgr;
    config_mgr.initialize();
    config_mgr.set_event_bus(&bus);

    LIZ_INFO("  ConfigurationManager initialized and connected to EventBus");

    std::cout << std::endl;

    // ==================================================================
    // 2. Create Default Configuration with 5 sections
    // ==================================================================
    LIZ_INFO("[2] Creating Default Configuration with sections...");

    auto* config = config_mgr.create_configuration("Default", "Default engine configuration");

    // Graphics section
    auto* graphics = config->create_section("Graphics");
    graphics->add_value(make_val("width",      liz::ConfigValueType::Int,    1920,  1920, "Render width"));
    graphics->add_value(make_val("height",     liz::ConfigValueType::Int,    1080,  1080, "Render height"));
    graphics->add_value(make_val("vsync",      liz::ConfigValueType::Bool,   true,   true,  "Vertical sync"));
    graphics->add_value(make_val("fullscreen", liz::ConfigValueType::Bool,   false,  false, "Fullscreen mode"));

    // Audio section
    auto* audio = config->create_section("Audio");
    audio->add_value(make_val("volume", liz::ConfigValueType::Int,  80, 80, "Master volume (0-100)"));
    audio->add_value(make_val("mute",   liz::ConfigValueType::Bool, false, false, "Mute toggle"));

    // GPU section
    auto* gpu = config->create_section("GPU");
    gpu->add_value(make_val("backend",      liz::ConfigValueType::String, std::string("CPU"),    std::string("CPU"),    "GPU backend"));
    gpu->add_value(make_val("memory_limit", liz::ConfigValueType::Int,    512,             512,             "VRAM limit (MB)"));

    // AI section
    auto* ai = config->create_section("AI");
    ai->add_value(make_val("batch_size", liz::ConfigValueType::Int,    4,             4,   "Batch size"));
    ai->add_value(make_val("precision",  liz::ConfigValueType::String, std::string("FP32"), std::string("FP32"), "Compute precision"));

    // Engine section
    auto* engine_sec = config->create_section("Engine");
    engine_sec->add_value(make_val("threads",  liz::ConfigValueType::Int,    4,             4,   "Worker threads"));
    engine_sec->add_value(make_val("language", liz::ConfigValueType::String, std::string("en"),    std::string("en"),    "UI language"));

    {
        std::ostringstream oss;
        oss << "  Configuration: " << config->to_string();
        LIZ_INFO(oss.str());
    }

    std::cout << std::endl;

    // ==================================================================
    // 3. Modify some values
    // ==================================================================
    LIZ_INFO("[3] Modifying values...");

    {
        auto* val = graphics->find_mutable("fullscreen");
        if (val) val->set_value(true);
    }
    {
        auto* val = audio->find_mutable("volume");
        if (val) val->set_value(50);
    }
    {
        auto* val = gpu->find_mutable("backend");
        if (val) val->set_value(std::string("CUDA"));
    }
    {
        auto* val = ai->find_mutable("precision");
        if (val) val->set_value(std::string("FP16"));
    }

    {
        std::ostringstream oss;
        oss << "  Modified: fullscreen=true, volume=50, backend=CUDA, precision=FP16";
        LIZ_INFO(oss.str());
    }

    std::cout << std::endl;

    // ==================================================================
    // 4. Show full configuration listing
    // ==================================================================
    LIZ_INFO("[4] Full configuration listing:");

    auto sections = config->list_sections();
    for (const auto& sec_name : sections) {
        auto* sec = config->find_section(sec_name);
        if (!sec) continue;

        std::ostringstream oss;
        oss << "  [" << sec_name << "] (" << sec->count() << " values)";
        LIZ_INFO(oss.str());

        auto keys = sec->list();
        for (const auto& key : keys) {
            auto* val = sec->find(key);
            if (val) {
                std::ostringstream oss2;
                oss2 << "    - " << val->to_string();
                LIZ_INFO(oss2.str());
            }
        }
    }

    std::cout << std::endl;

    // ==================================================================
    // 5. Show sections summary
    // ==================================================================
    LIZ_INFO("[5] Sections summary:");
    {
        auto secs = config->list_sections();
        std::ostringstream oss;
        oss << "  Total sections: " << secs.size();
        LIZ_INFO(oss.str());
        for (const auto& s : secs) {
            auto* sec = config->find_section(s);
            std::ostringstream oss2;
            oss2 << "    - " << s << ": " << sec->count() << " values, "
                 << sec->modified_count() << " modified";
            LIZ_INFO(oss2.str());
        }
    }

    std::cout << std::endl;

    // ==================================================================
    // 6. Show statistics
    // ==================================================================
    LIZ_INFO("[6] ConfigurationManager statistics:");
    {
        auto stats = config_mgr.statistics();
        std::ostringstream oss;
        oss << "  " << stats.to_string();
        LIZ_INFO(oss.str());
    }

    std::cout << std::endl;

    // ==================================================================
    // 7. Show EventBus statistics
    // ==================================================================
    LIZ_INFO("[7] EventBus statistics:");
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
    // 8. Diagnostics report with config data
    // ==================================================================
    LIZ_INFO("[8] Diagnostics report (updated with config metrics):");
    {
        liz::DiagnosticsDataProvider provider;
        provider.events_published   = bus.event_count();
        provider.services_registered = registry.count();

        auto cs = config_mgr.statistics();
        provider.configurations        = cs.created;
        provider.configuration_sections = cs.sections;
        provider.configuration_values  = cs.values;
        provider.modified_configuration_values = cs.modified_values;

        liz::DiagnosticsManager diag;
        diag.set_provider(&provider);
        diag.capture_snapshot("after_sprint22");

        auto engine_stats = diag.statistics();
        std::ostringstream oss;
        oss << "  " << engine_stats.to_string();
        LIZ_INFO(oss.str());

        diag.print_report();
    }

    std::cout << std::endl;

    // ==================================================================
    // 9. EngineAPI integration test
    // ==================================================================
    LIZ_INFO("[9] EngineAPI integration test:");
    {
        liz::EngineBuilder builder;
        builder.set_application_name("Sprint22Test")
               .set_application_version("1.0.0")
               .enable_gpu(false)
               .enable_diagnostics(false)
               .enable_events(true)
               .build();

        liz::EngineAPI api;
        auto result = api.initialize(builder);

        if (result == liz::ApiResult::Success) {
            // Create a config via API.
            liz::ConfigurationInfo info;
            api.create_configuration("TestConfig", info);
            {
                std::ostringstream oss;
                oss << "  API created config: " << info.name
                    << " uuid=" << info.uuid
                    << " active=" << (info.active ? "true" : "false");
                LIZ_INFO(oss.str());
            }

            // Get statistics.
            auto api_stats = api.configuration_statistics();
            {
                std::ostringstream oss;
                oss << "  API config statistics: " << api_stats.to_string();
                LIZ_INFO(oss.str());
            }

            // List configurations.
            auto configs = api.list_configurations();
            {
                std::ostringstream oss;
                oss << "  API configs listed: " << configs.size();
                LIZ_INFO(oss.str());
            }

            api.shutdown();
        } else {
            LIZ_WARN("  EngineAPI initialization failed");
        }
    }

    std::cout << std::endl;

    // ==================================================================
    // Cleanup
    // ==================================================================
    config_mgr.clear();
    config_mgr.shutdown();
    registry.clear();
    bus.unsubscribe(&listener);

    LIZ_INFO("Configuration System demo completed");
    return 0;
}
