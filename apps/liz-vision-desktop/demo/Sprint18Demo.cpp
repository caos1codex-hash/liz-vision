#include "demo/Sprint18Demo.h"

#include "engine/core/Logger.h"
#include "engine/events/EventBus.h"
#include "engine/events/Event.h"
#include "engine/events/EventListener.h"
#include "engine/events/EventType.h"
#include "engine/services/ServiceRegistry.h"
#include "engine/services/ServiceType.h"
#include "engine/diagnostics/DiagnosticsManager.h"
#include "engine/diagnostics/PerformanceSnapshot.h"
#include "engine/diagnostics/EngineStatistics.h"
#include "engine/pipeline/PipelineGraph.h"
#include "engine/pipeline/PipelineNode.h"
#include "engine/pipeline/PipelineTypes.h"
#include "engine/pipeline/PipelineExecutor.h"
#include "engine/pipeline/PipelineStatistics.h"
#include "engine/assets/AssetManager.h"
#include "engine/assets/AssetType.h"
#include "engine/project/ProjectManager.h"
#include "engine/project/Project.h"
#include "engine/project/ProjectTypes.h"
#include "engine/workspace/WorkspaceManager.h"
#include "engine/workspace/Workspace.h"
#include "engine/workspace/WorkspaceTypes.h"
#include "engine/workspace/WorkspaceLayout.h"
#include "engine/workspace/WorkspacePreferences.h"
#include "engine/workspace/WorkspaceStatistics.h"

#include <iostream>
#include <memory>
#include <sstream>

// -- Workspace Event Listener ------------------------------------------------
class WorkspaceEventListener : public liz::EventListener {
public:
    std::string_view name() const override { return "WorkspaceEventListener"; }

    void on_event(const liz::Event& event) override {
        auto t = event.type();
        if (t == liz::EventType::WorkspaceCreated ||
            t == liz::EventType::WorkspaceOpened ||
            t == liz::EventType::WorkspaceActivated ||
            t == liz::EventType::WorkspaceClosed ||
            t == liz::EventType::WorkspaceDestroyed ||
            t == liz::EventType::ProjectAddedToWorkspace ||
            t == liz::EventType::ProjectRemovedFromWorkspace) {
            std::ostringstream oss;
            oss << "  [WorkspaceEventListener] " << event.info();
            LIZ_INFO(oss.str());
        }
    }

private:
    using liz::EventListener::on_event;
};

// -- Demo implementation -------------------------------------------------------
int Sprint18Demo::run() {
    LIZ_INFO("--- Workspace Foundation (Sprint 18) ---");

    liz::EventBus bus;
    WorkspaceEventListener listener;
    bus.subscribe(&listener);

    std::cout << std::endl;

    // ==================================================================
    // 1. Create Workspace "LIZ Studio"
    // ==================================================================
    LIZ_INFO("[1] Creating workspace 'LIZ Studio'...");

    liz::WorkspaceManager ws_mgr;
    ws_mgr.initialize();
    ws_mgr.set_event_bus(&bus);

    auto* workspace = ws_mgr.create_workspace("LIZ Studio");
    if (!workspace) {
        LIZ_ERROR("Failed to create workspace");
        return 1;
    }

    {
        std::ostringstream oss;
        oss << "  " << workspace->to_string();
        LIZ_INFO(oss.str());
    }

    std::cout << std::endl;

    // ==================================================================
    // 2. Open Workspace
    // ==================================================================
    LIZ_INFO("[2] Opening workspace...");
    ws_mgr.open_workspace(workspace->uuid());
    ws_mgr.set_active_workspace(workspace->uuid());
    {
        std::ostringstream oss;
        oss << "  State: " << liz::workspace_state_to_string(workspace->state());
        LIZ_INFO(oss.str());
    }

    std::cout << std::endl;

    // ==================================================================
    // 3. Create two projects
    // ==================================================================
    LIZ_INFO("[3] Creating projects...");

    liz::ProjectManager proj_mgr;
    proj_mgr.initialize();
    proj_mgr.set_event_bus(&bus);

    auto* proj_video = proj_mgr.create_project("Proyecto Video");
    auto* proj_anime = proj_mgr.create_project("Proyecto Anime");

    if (proj_video) proj_video->set_author("LIZ Team");
    if (proj_anime) proj_anime->set_author("LIZ Team");

    {
        std::ostringstream oss;
        oss << "  " << (proj_video ? proj_video->to_string() : "(null)");
        LIZ_INFO(oss.str());
    }
    {
        std::ostringstream oss;
        oss << "  " << (proj_anime ? proj_anime->to_string() : "(null)");
        LIZ_INFO(oss.str());
    }

    std::cout << std::endl;

    // ==================================================================
    // 4. Add both projects to workspace
    // ==================================================================
    LIZ_INFO("[4] Adding projects to workspace...");

    if (proj_video && proj_anime) {
        ws_mgr.add_project(workspace->uuid(), proj_video->uuid());
        ws_mgr.add_project(workspace->uuid(), proj_anime->uuid());
    }

    {
        auto projects = ws_mgr.list_projects(workspace->uuid());
        std::ostringstream oss;
        oss << "  Projects in workspace: " << projects.size();
        for (const auto& uuid : projects) {
            oss << " [" << uuid << "]";
        }
        LIZ_INFO(oss.str());
    }

    std::cout << std::endl;

    // ==================================================================
    // 5. Create simulated assets
    // ==================================================================
    LIZ_INFO("[5] Creating simulated assets...");

    liz::AssetManager asset_mgr;
    asset_mgr.initialize();
    asset_mgr.set_event_bus(&bus);

    asset_mgr.create_asset("video_input.mp4", liz::AssetType::Video, 1024 * 1024 * 50);
    asset_mgr.create_asset("anime_input.mp4", liz::AssetType::Video, 1024 * 1024 * 30);
    asset_mgr.create_asset("liz_upscaler.bin", liz::AssetType::Model, 1024 * 1024 * 200);
    asset_mgr.load("video_input.mp4");
    asset_mgr.load("anime_input.mp4");
    asset_mgr.load("liz_upscaler.bin");

    {
        auto asset_stats = asset_mgr.statistics();
        std::ostringstream oss;
        oss << "  Assets: " << asset_stats.total_created
            << " total, " << asset_stats.active_assets << " active";
        LIZ_INFO(oss.str());
    }

    std::cout << std::endl;

    // ==================================================================
    // 6. Create a pipeline graph
    // ==================================================================
    LIZ_INFO("[6] Creating pipeline graph...");

    liz::PipelineExecutor executor;
    executor.initialize();
    executor.set_event_bus(&bus);

    auto graph = std::make_unique<liz::PipelineGraph>("Studio Pipeline");
    graph->set_event_bus(&bus);

    auto* input   = graph->create_node("Input",   liz::PipelineNodeType::Input);
    auto* decoder = graph->create_node("Decoder", liz::PipelineNodeType::Decoder);
    auto* tensor  = graph->create_node("Tensor",  liz::PipelineNodeType::TensorConverter);
    auto* infer   = graph->create_node("LIZ AI", liz::PipelineNodeType::Inference);
    auto* output  = graph->create_node("Output",  liz::PipelineNodeType::Output);

    graph->connect(input->uuid(),   decoder->uuid(), liz::ConnectionType::Data);
    graph->connect(decoder->uuid(), tensor->uuid(),  liz::ConnectionType::Data);
    graph->connect(tensor->uuid(),  infer->uuid(),   liz::ConnectionType::Data);
    graph->connect(infer->uuid(),   output->uuid(),  liz::ConnectionType::Data);

    executor.register_pipeline(std::move(graph));
    executor.execute("Studio Pipeline");

    {
        std::ostringstream oss;
        oss << "  Pipelines: " << executor.pipeline_count();
        LIZ_INFO(oss.str());
    }

    std::cout << std::endl;

    // ==================================================================
    // 7. Modify preferences
    // ==================================================================
    LIZ_INFO("[7] Modifying workspace preferences...");

    workspace->preferences().set_theme("midnight-blue");
    workspace->preferences().set_language("es");
    workspace->preferences().set_autosave_enabled(true);
    workspace->preferences().set_gpu_enabled(true);
    workspace->preferences().set_diagnostics_enabled(true);
    workspace->preferences().set_auto_validate_pipeline(true);
    workspace->preferences().set_thread_count(8);
    workspace->preferences().set_batch_size(16);
    workspace->preferences().set_recent_projects_limit(20);

    {
        std::ostringstream oss;
        oss << "  " << workspace->preferences().to_string();
        LIZ_INFO(oss.str());
    }

    std::cout << std::endl;

    // ==================================================================
    // 8. Modify layout
    // ==================================================================
    LIZ_INFO("[8] Modifying workspace layout...");

    workspace->layout().add_panel("Timeline");
    workspace->layout().add_panel("Preview");
    workspace->layout().add_panel("Properties");
    workspace->layout().add_panel("Assets");
    workspace->layout().set_active_panel("Preview");
    workspace->layout().set_position(0, 0);
    workspace->layout().set_size(3840, 2160);

    {
        std::ostringstream oss;
        oss << "  " << workspace->layout().to_string();
        LIZ_INFO(oss.str());
    }

    std::cout << std::endl;

    // ==================================================================
    // 9. Show statistics
    // ==================================================================
    LIZ_INFO("[9] Workspace system statistics:");
    {
        auto stats = ws_mgr.statistics();
        std::ostringstream oss;
        oss << "  " << stats.to_string();
        LIZ_INFO(oss.str());
    }

    std::cout << std::endl;

    // ==================================================================
    // 10. Show events published
    // ==================================================================
    LIZ_INFO("[10] Events published:");
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
    // 11. Diagnostics report with workspace data
    // ==================================================================
    LIZ_INFO("[11] Diagnostics report (updated):");
    {
        liz::DiagnosticsDataProvider provider;
        provider.events_published = bus.event_count();
        provider.assets_active = asset_mgr.statistics().active_assets;
        provider.pipelines_active = executor.pipeline_count();
        provider.pipeline_nodes = 5;

        auto proj_stats = proj_mgr.statistics();
        provider.active_project = proj_stats.active_project;
        provider.open_projects  = proj_stats.projects_open;
        provider.saved_projects = proj_stats.projects_saved;

        auto ws_stats = ws_mgr.statistics();
        provider.active_workspace   = ws_stats.active_workspace;
        provider.loaded_workspaces  = ws_stats.workspaces_open;
        provider.workspace_projects = ws_stats.projects_loaded;

        liz::DiagnosticsManager diag;
        diag.set_provider(&provider);
        diag.capture_snapshot("after_sprint18");

        auto engine_stats = diag.statistics();
        std::ostringstream oss;
        oss << "  " << engine_stats.to_string();
        LIZ_INFO(oss.str());

        diag.print_report();
    }

    std::cout << std::endl;

    // ==================================================================
    // 12. Close Workspace
    // ==================================================================
    LIZ_INFO("[12] Closing workspace...");
    ws_mgr.close_workspace(workspace->uuid());
    {
        std::ostringstream oss;
        oss << "  State: " << liz::workspace_state_to_string(workspace->state());
        LIZ_INFO(oss.str());
    }

    std::cout << std::endl;

    // ==================================================================
    // Cleanup
    // ==================================================================
    executor.clear();
    executor.shutdown();
    asset_mgr.shutdown();
    ws_mgr.clear();
    ws_mgr.shutdown();
    proj_mgr.clear();
    proj_mgr.shutdown();
    bus.unsubscribe(&listener);

    LIZ_INFO("Workspace demo completed");
    return 0;
}
