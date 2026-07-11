#include "demo/Sprint23Demo.h"

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
#include "engine/config/ConfigSchema.h"
#include "engine/config/ConfigOverride.h"
#include "engine/config/ConfigurationProfile.h"
#include "engine/config/AdvancedConfigurationManager.h"
#include "engine/config/ConfigPersistence.h"
#include "engine/api/EngineAPI.h"
#include "engine/api/ApiTypes.h"
#include "engine/api/EngineBuilder.h"

#include <iostream>
#include <memory>
#include <sstream>
#include <vector>

// -- Advanced Config Event Listener ----------------------------------------
class AdvancedConfigEventListener : public liz::EventListener {
public:
    std::string_view name() const override { return "AdvancedConfigEventListener"; }

    void on_event(const liz::Event& event) override {
        auto t = event.type();
        if (t == liz::EventType::ConfigSchemaValidated ||
            t == liz::EventType::ConfigValidationFailed ||
            t == liz::EventType::ConfigReloaded ||
            t == liz::EventType::ConfigProfileActivated ||
            t == liz::EventType::ConfigOverrideApplied ||
            t == liz::EventType::ConfigurationCreated ||
            t == liz::EventType::ConfigurationActivated) {
            std::ostringstream oss;
            oss << "  [AdvancedConfigEventListener] " << event.info();
            LIZ_INFO(oss.str());
        }
    }

private:
    using liz::EventListener::on_event;
};

static std::unique_ptr<liz::ConfigValue> make_val(
    const std::string& key, liz::ConfigValueType type,
    liz::ConfigValue::ValueVariant value,
    liz::ConfigValue::ValueVariant default_val = {},
    const std::string& desc = "")
{
    return std::make_unique<liz::ConfigValue>(key, type, value, default_val, desc);
}

int Sprint23Demo::run() {
    LIZ_INFO("--- Advanced Configuration System (Sprint 23) ---");

    liz::EventBus bus;
    AdvancedConfigEventListener listener;
    bus.subscribe(&listener);

    liz::ServiceRegistry registry;

    std::cout << std::endl;

    // ==================================================================
    // 1. Load base configuration (reusing Foundation ConfigurationManager)
    // ==================================================================
    LIZ_INFO("[1] Loading base configuration...");
    {
        liz::ConfigurationManager foundation;
        foundation.initialize();
        foundation.set_event_bus(&bus);

        auto* config = foundation.create_configuration("Base", "Base engine configuration");
        auto* graphics = config->create_section("Graphics");
        graphics->add_value(make_val("width",      liz::ConfigValueType::Int,  1920, 1920, "Render width"));
        graphics->add_value(make_val("height",     liz::ConfigValueType::Int,  1080, 1080, "Render height"));
        graphics->add_value(make_val("vsync",      liz::ConfigValueType::Bool, true,  true,  "Vertical sync"));
        auto* gpu = config->create_section("GPU");
        gpu->add_value(make_val("backend",  liz::ConfigValueType::String, std::string("CPU"), std::string("CPU"), "GPU backend"));
        gpu->add_value(make_val("memory_mb", liz::ConfigValueType::Int, 512, 512, "VRAM limit (MB)"));

        std::ostringstream oss;
        oss << "  Base config: " << config->section_count() << " sections, "
            << config->total_values() << " values";
        LIZ_INFO(oss.str());

        foundation.clear();
        foundation.shutdown();
    }

    std::cout << std::endl;

    // ==================================================================
    // Full Advanced demo on a single owned stack of managers.
    // ==================================================================
    liz::ConfigurationManager config_mgr;
    config_mgr.initialize();
    config_mgr.set_event_bus(&bus);
    registry.register_service("ConfigurationManager", liz::ServiceType::ConfigurationManager, "1.0.0");

    liz::AdvancedConfigurationManager advanced;
    advanced.bind(&config_mgr);
    advanced.set_event_bus(&bus);
    advanced.initialize();

    // ==================================================================
    // 2. Create schema with validation rules.
    // ==================================================================
    LIZ_INFO("[2] Creating ConfigSchema with rules...");
    {
        auto* gfx_schema = advanced.schema().create_section("Graphics");
        auto r1 = std::make_unique<liz::ConfigSchemaRule>("width", liz::ConfigValueType::Int);
        r1->required(true).min_value(320).max_value(7680);
        gfx_schema->add_rule(std::move(r1));
        auto r2 = std::make_unique<liz::ConfigSchemaRule>("height", liz::ConfigValueType::Int);
        r2->required(true).min_value(240).max_value(4320);
        gfx_schema->add_rule(std::move(r2));
        auto r3 = std::make_unique<liz::ConfigSchemaRule>("vsync", liz::ConfigValueType::Bool);
        r3->required(false);
        gfx_schema->add_rule(std::move(r3));

        auto* gpu_schema = advanced.schema().create_section("GPU");
        auto rg = std::make_unique<liz::ConfigSchemaRule>("backend", liz::ConfigValueType::String);
        rg->required(true).allowed_values({std::string("CPU"), std::string("CUDA"), std::string("DirectX"), std::string("Vulkan")});
        gpu_schema->add_rule(std::move(rg));
        auto rm = std::make_unique<liz::ConfigSchemaRule>("memory_mb", liz::ConfigValueType::Int);
        rm->required(true).min_value(64).max_value(32768);
        gpu_schema->add_rule(std::move(rm));

        std::ostringstream oss;
        oss << "  Schema sections: " << advanced.schema().section_count();
        LIZ_INFO(oss.str());
    }

    std::cout << std::endl;

    // ==================================================================
    // 3. Validate a CORRECT configuration -> ConfigSchemaValidated
    // ==================================================================
    LIZ_INFO("[3] Validating a CORRECT configuration...");
    {
        auto* good = config_mgr.create_configuration("GoodConfig", "Should pass schema");
        auto* gfx = good->create_section("Graphics");
        gfx->add_value(make_val("width",  liz::ConfigValueType::Int, 1920, 1920, "Render width"));
        gfx->add_value(make_val("height", liz::ConfigValueType::Int, 1080, 1080, "Render height"));
        gfx->add_value(make_val("vsync",  liz::ConfigValueType::Bool, true, true, "VSync"));
        auto* gpu = good->create_section("GPU");
        gpu->add_value(make_val("backend",  liz::ConfigValueType::String, std::string("CUDA"), std::string("CPU"), "Backend"));
        gpu->add_value(make_val("memory_mb", liz::ConfigValueType::Int, 1024, 512, "VRAM limit"));

        // Validate by UUID so we can also exercise the non-active path.
        auto result = advanced.validate(good->uuid());
        std::ostringstream oss;
        oss << "  Validation valid=" << (result.valid ? "true" : "false")
            << " errors=" << result.error_count();
        LIZ_INFO(oss.str());
        if (!result.valid) {
            for (const auto& e : result.errors) {
                std::ostringstream eoss;
                eoss << "    - " << e.to_string();
                LIZ_INFO(eoss.str());
            }
        }
    }

    std::cout << std::endl;

    // ==================================================================
    // 4. Detect an INVALID configuration -> ConfigValidationFailed
    // ==================================================================
    LIZ_INFO("[4] Detecting an INVALID configuration...");
    {
        auto* bad = config_mgr.create_configuration("BadConfig", "Should fail schema");
        auto* gfx = bad->create_section("Graphics");
        gfx->add_value(make_val("width",  liz::ConfigValueType::Int, 99999, 1920, "Out of range width"));
        // 'height' intentionally missing -> required rule fails
        auto* gpu = bad->create_section("GPU");
        gpu->add_value(make_val("backend",  liz::ConfigValueType::String, std::string("MetalX"), std::string("CPU"), "Invalid backend"));
        gpu->add_value(make_val("memory_mb", liz::ConfigValueType::Int, 8, 512, "Too low memory"));

        auto result = advanced.validate(bad->uuid());
        std::ostringstream oss;
        oss << "  Validation valid=" << (result.valid ? "true" : "false")
            << " errors=" << result.error_count();
        LIZ_INFO(oss.str());
        for (const auto& e : result.errors) {
            std::ostringstream eoss;
            eoss << "    - " << e.to_string();
            LIZ_INFO(eoss.str());
        }
    }

    std::cout << std::endl;

    // ==================================================================
    // 5. Apply a runtime override -> ConfigOverrideApplied
    // ==================================================================
    LIZ_INFO("[5] Applying a runtime override...");
    {
        // Activate the GoodConfig so overrides can resolve onto it.
        config_mgr.set_active(config_mgr.list().front());
        advanced.apply_runtime_override("GPU", "memory_mb", liz::ConfigValue::ValueVariant(2048));
        advanced.apply_runtime_override("GPU", "backend",   liz::ConfigValue::ValueVariant(std::string("Vulkan")));

        auto* active = config_mgr.active();
        if (active) {
            auto* gpu = active->find_section("GPU");
            if (gpu) {
                auto* mem = gpu->find("memory_mb");
                auto* bk  = gpu->find("backend");
                if (mem && bk) {
                    std::ostringstream oss;
                    oss << "  After override: backend=" << bk->as_string()
                        << " memory_mb=" << mem->as_int();
                    LIZ_INFO(oss.str());
                } else {
                    LIZ_INFO("  After override: (GPU section present, memory_mb/backend missing)");
                }
            } else {
                LIZ_INFO("  After override: (active config has no GPU section)");
            }
        }
    }

    std::cout << std::endl;

    // ==================================================================
    // 6. Switch profiles: Development -> Production -> Benchmark
    // ==================================================================
    LIZ_INFO("[6] Switching execution profiles...");
    {
        auto* dev = advanced.create_profiled("DevProfile",  liz::ConfigurationProfile::Development, "");
        auto* prod = advanced.create_profiled("ProdProfile", liz::ConfigurationProfile::Production,  "");
        auto* bench = advanced.create_profiled("BenchProfile", liz::ConfigurationProfile::Benchmark, "");

        (void)dev; (void)prod; (void)bench; // bound by create_profiled

        auto print_active = [&]() {
            std::ostringstream oss;
            oss << "  Active profile: " << liz::configuration_profile_to_string(advanced.active_profile());
            LIZ_INFO(oss.str());
        };

        advanced.activate_profile(liz::ConfigurationProfile::Development); print_active();
        advanced.activate_profile(liz::ConfigurationProfile::Production);  print_active();
        advanced.activate_profile(liz::ConfigurationProfile::Benchmark);   print_active();
    }

    std::cout << std::endl;

    // ==================================================================
    // 7. Reload the active configuration without restarting the engine.
    // ==================================================================
    LIZ_INFO("[7] Dynamic reload (without restarting engine)...");
    {
        // Look up a configuration that actually has sections and a bound path.
        // After the profile switches the active config is the last-created
        // (empty) profile, so we (re)bind GoodConfig, activate it, save,
        // mutate, then reload.
        std::string good_uuid;
        for (const auto& uuid : config_mgr.list()) {
            auto* c = config_mgr.find(uuid);
            if (c && c->name() == "GoodConfig") { good_uuid = uuid; break; }
        }

        if (!good_uuid.empty()) {
            std::string path = "sprint23_reload_demo.cfg";
            advanced.bind_profile(good_uuid, liz::ConfigurationProfile::Custom, path);
            config_mgr.set_active(good_uuid);

            auto* active = config_mgr.active();
            liz::ConfigPersistence::save_to(path, *active);

            // Mutate in memory to prove the reload restores on-disk state.
            auto* gpu_before = active ? active->find_section("GPU") : nullptr;
            if (gpu_before) {
                if (auto* v = gpu_before->find_mutable("memory_mb")) v->set_value(16);
            }
            {
                std::ostringstream oss;
                oss << "  Before reload: memory_mb=";
                if (gpu_before && gpu_before->find("memory_mb")) {
                    oss << gpu_before->find("memory_mb")->as_int();
                } else { oss << "(n/a)"; }
                LIZ_INFO(oss.str());
            }

            advanced.reload_active();

            auto* gpu_after = active ? active->find_section("GPU") : nullptr;
            {
                std::ostringstream oss;
                oss << "  After reload:  memory_mb=";
                if (gpu_after && gpu_after->find("memory_mb")) {
                    oss << gpu_after->find("memory_mb")->as_int();
                } else { oss << "(n/a)"; }
                LIZ_INFO(oss.str());
            }
        } else {
            LIZ_WARN("  GoodConfig not found; skipping reload demonstration");
        }
    }

    std::cout << std::endl;

    // ==================================================================
    // 8. Show statistics in Diagnostics
    // ==================================================================
    LIZ_INFO("[8] Diagnostics with advanced configuration metrics...");
    {
        auto cs = advanced.statistics();
        std::ostringstream oss;
        oss << "  Advanced stats: " << cs.to_string();
        LIZ_INFO(oss.str());

        liz::DiagnosticsDataProvider provider;
        provider.events_published    = bus.event_count();
        provider.services_registered = registry.count();

        provider.configurations        = cs.created;
        provider.configuration_sections = cs.sections;
        provider.configuration_values  = cs.values;
        provider.modified_configuration_values = cs.modified_values;

        provider.config_schema_validations  = cs.schema_validations;
        provider.config_reload_count        = cs.reload_count;
        provider.config_override_count      = cs.override_count;
        provider.config_profile_switches    = cs.profile_switches;
        provider.config_validation_failures = cs.validation_failures;

        liz::DiagnosticsManager diag;
        diag.set_provider(&provider);
        diag.capture_snapshot("after_sprint23");

        auto engine_stats = diag.statistics();
        std::ostringstream eoss;
        eoss << "  " << engine_stats.to_string();
        LIZ_INFO(eoss.str());
    }

    std::cout << std::endl;

    // ==================================================================
    // EngineAPI Advanced facade integration
    // ==================================================================
    LIZ_INFO("[*] EngineAPI Advanced facade integration:");
    {
        liz::EngineBuilder builder;
        builder.set_application_name("Sprint23Test")
               .set_application_version("1.0.0")
               .enable_gpu(false)
               .enable_diagnostics(false)
               .enable_events(true)
               .build();

        liz::EngineAPI api;
        if (api.initialize(builder) == liz::ApiResult::Success) {
            liz::ConfigurationInfo info;
            api.create_profiled_configuration("ApiProfiled", "Benchmark", "", info);
            {
                std::ostringstream oss;
                oss << "  API profiled config: " << info.name << " uuid=" << info.uuid;
                LIZ_INFO(oss.str());
            }
            api.activate_configuration_profile("Benchmark");
            api.apply_config_override("GPU", "memory_mb", "8192");

            auto summary = api.configuration_schema_summary();
            auto astat   = api.advanced_configuration_statistics();
            std::ostringstream oss;
            oss << "  API schema summary: " << summary.to_string()
                << " | advanced stats: " << astat.to_string();
            LIZ_INFO(oss.str());

            api.shutdown();
        } else {
            LIZ_WARN("  EngineAPI initialization failed");
        }
    }

    std::cout << std::endl;

    // ==================================================================
    // Cleanup
    // ==================================================================
    advanced.shutdown();
    config_mgr.clear();
    config_mgr.shutdown();
    registry.clear();
    bus.unsubscribe(&listener);

    LIZ_INFO("Advanced Configuration System demo completed");
    return 0;
}
