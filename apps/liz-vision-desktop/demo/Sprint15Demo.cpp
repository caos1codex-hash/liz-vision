#include "demo/Sprint15Demo.h"

#include "engine/api/EngineAPI.h"
#include "engine/api/EngineBuilder.h"
#include "engine/api/EngineSession.h"
#include "engine/api/ApiResult.h"
#include "engine/api/ApiTypes.h"
#include "engine/core/Logger.h"

#include <iostream>
#include <sstream>

int Sprint15Demo::run() {
    LIZ_INFO("--- Public API Foundation (Sprint 15) ---");

    // ════════════════════════════════════════════════════════════════════
    // 1. Build engine configuration.
    // ════════════════════════════════════════════════════════════════════
    LIZ_INFO("[1] Creating EngineBuilder and configuring options...");

    liz::EngineBuilder builder;
    builder.set_application_name("LIZ Vision Studio")
           .set_application_version("0.1.0")
           .enable_gpu(false)
           .enable_plugins(false)
           .enable_diagnostics(true)
           .enable_assets(true)
           .enable_events(true)
           .build();

    std::cout << std::endl;

    // ════════════════════════════════════════════════════════════════════
    // 2. Initialize the API.
    // ════════════════════════════════════════════════════════════════════
    LIZ_INFO("[2] Initializing EngineAPI...");

    liz::EngineAPI api;
    auto result = api.initialize(builder);

    {
        std::ostringstream oss;
        oss << "  initialize() -> " << liz::api_result_to_string(result);
        LIZ_INFO(oss.str());
    }
    {
        std::ostringstream oss;
        oss << "  is_initialized() -> " << (api.is_initialized() ? "true" : "false");
        LIZ_INFO(oss.str());
    }

    std::cout << std::endl;

    // ════════════════════════════════════════════════════════════════════
    // 3. Query API version.
    // ════════════════════════════════════════════════════════════════════
    LIZ_INFO("[3] Querying API version...");
    {
        auto ver = api.version();
        std::ostringstream oss;
        oss << "  version=" << ver.version_string()
            << " full=" << ver.full_version_string()
            << " major=" << ver.major()
            << " minor=" << ver.minor()
            << " patch=" << ver.patch()
            << " build=" << ver.build();
        LIZ_INFO(oss.str());
    }

    std::cout << std::endl;

    // ════════════════════════════════════════════════════════════════════
    // 4. Create a session.
    // ════════════════════════════════════════════════════════════════════
    LIZ_INFO("[4] Creating a session...");
    liz::SessionInfo session_info;
    result = api.create_session(session_info);

    {
        std::ostringstream oss;
        oss << "  create_session() -> " << liz::api_result_to_string(result)
            << " id=" << session_info.session_id
            << " state=" << session_info.state;
        LIZ_INFO(oss.str());
    }

    std::cout << std::endl;

    // ════════════════════════════════════════════════════════════════════
    // 5. Query engine info.
    // ════════════════════════════════════════════════════════════════════
    LIZ_INFO("[5] Querying engine info...");
    {
        auto info = api.engine_info();
        std::ostringstream oss;
        oss << "  session_id=" << info.session_id
            << " state=" << info.state
            << " app=" << info.application_name
            << " app_ver=" << info.application_version
            << " engine_ver=" << info.engine_version
            << " gpu=" << (info.gpu_enabled ? "on" : "off")
            << " diag=" << (info.diagnostics_enabled ? "on" : "off")
            << " assets=" << (info.assets_enabled ? "on" : "off")
            << " events=" << (info.events_enabled ? "on" : "off");
        LIZ_INFO(oss.str());
    }

    std::cout << std::endl;

    // ════════════════════════════════════════════════════════════════════
    // 6. Query statistics.
    // ════════════════════════════════════════════════════════════════════
    LIZ_INFO("[6] Querying statistics...");
    {
        auto stats = api.statistics();
        std::ostringstream oss;
        oss << "  " << stats.to_string();
        LIZ_INFO(oss.str());
    }

    std::cout << std::endl;

    // ════════════════════════════════════════════════════════════════════
    // 7. Query service info.
    // ════════════════════════════════════════════════════════════════════
    LIZ_INFO("[7] Querying service info...");
    {
        auto services = api.service_info();
        std::ostringstream oss;
        oss << "  Total services: " << services.size();
        LIZ_INFO(oss.str());

        for (const auto& svc : services) {
            std::ostringstream oss2;
            oss2 << "    " << svc.name
                 << " type=" << svc.type
                 << " state=" << svc.state
                 << " version=" << svc.version;
            LIZ_INFO(oss2.str());
        }
    }

    std::cout << std::endl;

    // ════════════════════════════════════════════════════════════════════
    // 8. Query asset info.
    // ════════════════════════════════════════════════════════════════════
    LIZ_INFO("[8] Querying asset info...");
    {
        auto assets = api.asset_info();
        std::ostringstream oss;
        oss << "  Total assets: " << assets.size();
        LIZ_INFO(oss.str());

        for (const auto& asset : assets) {
            std::ostringstream oss2;
            oss2 << "    " << asset.name
                 << " type=" << asset.type
                 << " state=" << asset.state
                 << " version=" << asset.version
                 << " size=" << asset.size_bytes << " bytes";
            LIZ_INFO(oss2.str());
        }
    }

    std::cout << std::endl;

    // ════════════════════════════════════════════════════════════════════
    // 9. Query diagnostics info.
    // ════════════════════════════════════════════════════════════════════
    LIZ_INFO("[9] Querying diagnostics info...");
    {
        auto diag = api.diagnostics_info();
        std::ostringstream oss;
        oss << "  Snapshots: " << diag.snapshot_count;
        LIZ_INFO(oss.str());

        if (!diag.statistics_summary.empty()) {
            std::ostringstream oss2;
            oss2 << "  Statistics: " << diag.statistics_summary;
            LIZ_INFO(oss2.str());
        }
    }

    std::cout << std::endl;

    // ════════════════════════════════════════════════════════════════════
    // 10. List sessions.
    // ════════════════════════════════════════════════════════════════════
    LIZ_INFO("[10] Listing sessions...");
    {
        auto sessions = api.list_sessions();
        std::ostringstream oss;
        oss << "  Active sessions: " << sessions.size();
        LIZ_INFO(oss.str());

        for (const auto& s : sessions) {
            std::ostringstream oss2;
            oss2 << "    id=" << s.session_id
                 << " state=" << s.state
                 << " uptime=" << s.uptime_ms << "ms";
            LIZ_INFO(oss2.str());
        }
    }

    std::cout << std::endl;

    // ════════════════════════════════════════════════════════════════════
    // 11. Create a second session.
    // ════════════════════════════════════════════════════════════════════
    LIZ_INFO("[11] Creating a second session...");
    {
        liz::SessionInfo session2;
        auto r = api.create_session(session2);
        std::ostringstream oss;
        oss << "  create_session() -> " << liz::api_result_to_string(r)
            << " id=" << session2.session_id;
        LIZ_INFO(oss.str());
    }

    std::cout << std::endl;

    // ════════════════════════════════════════════════════════════════════
    // 12. Destroy first session.
    // ════════════════════════════════════════════════════════════════════
    LIZ_INFO("[12] Destroying first session...");
    {
        auto r = api.destroy_session(session_info.session_id);
        std::ostringstream oss;
        oss << "  destroy_session('" << session_info.session_id
            << "') -> " << liz::api_result_to_string(r);
        LIZ_INFO(oss.str());
    }

    std::cout << std::endl;

    // ════════════════════════════════════════════════════════════════════
    // 13. Test error handling: destroy nonexistent session.
    // ════════════════════════════════════════════════════════════════════
    LIZ_INFO("[13] Testing error handling...");
    {
        auto r = api.destroy_session("nonexistent_session");
        std::ostringstream oss;
        oss << "  destroy_session('nonexistent') -> "
            << liz::api_result_to_string(r);
        LIZ_INFO(oss.str());
    }

    std::cout << std::endl;

    // ════════════════════════════════════════════════════════════════════
    // 14. Version compatibility check.
    // ════════════════════════════════════════════════════════════════════
    LIZ_INFO("[14] Version compatibility check...");
    {
        auto current = api.version();
        liz::ApiVersion other{1, 0, 0, 0};

        std::ostringstream oss;
        oss << "  current " << current.version_string()
            << " compatible_with " << other.version_string()
            << " -> " << (current.compatible_with(other) ? "true" : "false");
        LIZ_INFO(oss.str());

        liz::ApiVersion incompatible{2, 0, 0, 0};
        std::ostringstream oss2;
        oss2 << "  current " << current.version_string()
             << " compatible_with " << incompatible.version_string()
             << " -> " << (current.compatible_with(incompatible) ? "true" : "false");
        LIZ_INFO(oss2.str());
    }

    std::cout << std::endl;

    // ════════════════════════════════════════════════════════════════════
    // 15. Shutdown.
    // ════════════════════════════════════════════════════════════════════
    LIZ_INFO("[15] Shutting down EngineAPI...");
    result = api.shutdown();

    {
        std::ostringstream oss;
        oss << "  shutdown() -> " << liz::api_result_to_string(result);
        LIZ_INFO(oss.str());
    }

    {
        std::ostringstream oss;
        oss << "  is_initialized() -> " << (api.is_initialized() ? "true" : "false");
        LIZ_INFO(oss.str());
    }

    std::cout << std::endl;

    LIZ_INFO("Public API demo completed");
    return 0;
}
