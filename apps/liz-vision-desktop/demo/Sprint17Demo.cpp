#include "demo/Sprint17Demo.h"

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
#include "engine/project/ProjectMetadata.h"
#include "engine/project/ProjectSettings.h"
#include "engine/project/ProjectStatistics.h"

#include <iostream>
#include <memory>
#include <sstream>

// -- Project Event Listener ---------------------------------------------------
class ProjectEventListener : public liz::EventListener {
public:
    std::string_view name() const override { return "ProjectEventListener"; }

    void on_event(const liz::Event& event) override {
        auto t = event.type();
        if (t == liz::EventType::ProjectCreated ||
            t == liz::EventType::ProjectOpened ||
            t == liz::EventType::ProjectSaved ||
            t == liz::EventType::ProjectClosed ||
            t == liz::EventType::ProjectDestroyed) {
            std::ostringstream oss;
            oss << "  [ProjectEventListener] " << event.info();
            LIZ_INFO(oss.str());
        }
    }

private:
    using liz::EventListener::on_event;
};

// -- Demo implementation -------------------------------------------------------
int Sprint17Demo::run() {
    LIZ_INFO("--- Project System Foundation (Sprint 17) ---");

    liz::EventBus bus;
    ProjectEventListener listener;
    bus.subscribe(&listener);

    std::cout << std::endl;

    // ═════════════════════════════════════════════════════════════════════
    // 1. Create ProjectManager
    // ═════════════════════════════════════════════════════════════════════
    LIZ_INFO("[1] Initializing ProjectManager...");

    liz::ProjectManager proj_mgr;
    proj_mgr.initialize();
    proj_mgr.set_event_bus(&bus);

    std::cout << std::endl;

    // ═════════════════════════════════════════════════════════════════════
    // 2. Create project
    // ═════════════════════════════════════════════════════════════════════
    LIZ_INFO("[2] Creating project 'Proyecto Demo'...");

    auto* project = proj_mgr.create_project("Proyecto Demo");
    if (!project) {
        LIZ_ERROR("Failed to create project");
        return 1;
    }

    project->set_description("Demo project for LIZ Vision Sprint 17");
    project->set_author("LIZ Team");
    project->set_version("1.0.0");

    {
        std::ostringstream oss;
        oss << "  " << project->to_string();
        LIZ_INFO(oss.str());
    }

    std::cout << std::endl;

    // ═════════════════════════════════════════════════════════════════════
    // 3. Modify metadata
    // ═════════════════════════════════════════════════════════════════════
    LIZ_INFO("[3] Modifying project metadata...");

    project->metadata().set_company("LIZ Vision Labs");
    project->metadata().set_website("https://liz-vision.example.com");
    project->metadata().set_author("LIZ Implementation Team");
    project->metadata().set_version("1.0.0");
    project->metadata().set_description("AI Video Enhancement Project");
    project->metadata().add_tag("video");
    project->metadata().add_tag("ai");
    project->metadata().add_tag("upscale");

    {
        std::ostringstream oss;
        oss << "  " << project->metadata().to_string();
        LIZ_INFO(oss.str());
    }

    std::cout << std::endl;

    // ═════════════════════════════════════════════════════════════════════
    // 4. Modify settings
    // ═════════════════════════════════════════════════════════════════════
    LIZ_INFO("[4] Modifying project settings...");

    project->settings().set_gpu_enabled(true);
    project->settings().set_diagnostics_enabled(true);
    project->settings().set_pipeline_auto_validation(true);
    project->settings().set_autosave_enabled(false);
    project->settings().set_thread_count(8);
    project->settings().set_language("es");
    project->settings().set_theme("dark");
    project->settings().set_batch_size(16);

    {
        std::ostringstream oss;
        oss << "  " << project->settings().to_string();
        LIZ_INFO(oss.str());
    }

    std::cout << std::endl;

    // ═════════════════════════════════════════════════════════════════════
    // 5. Create simulated assets
    // ═════════════════════════════════════════════════════════════════════
    LIZ_INFO("[5] Creating simulated assets...");

    liz::AssetManager asset_mgr;
    asset_mgr.initialize();
    asset_mgr.set_event_bus(&bus);

    asset_mgr.create_asset("input_video.mp4",  liz::AssetType::Video,  1024 * 1024 * 50);
    asset_mgr.create_asset("ai_model_liz.bin", liz::AssetType::Model,  1024 * 1024 * 200);
    asset_mgr.create_asset("output_video.mp4", liz::AssetType::Video,  0);
    asset_mgr.load("input_video.mp4");
    asset_mgr.load("ai_model_liz.bin");

    {
        auto asset_stats = asset_mgr.statistics();
        std::ostringstream oss;
        oss << "  Assets: " << asset_stats.total_created
            << " total, " << asset_stats.active_assets << " active";
        LIZ_INFO(oss.str());
    }

    std::cout << std::endl;

    // ═════════════════════════════════════════════════════════════════════
    // 6. Create a pipeline
    // ═════════════════════════════════════════════════════════════════════
    LIZ_INFO("[6] Creating pipeline...");

    liz::PipelineExecutor executor;
    executor.initialize();
    executor.set_event_bus(&bus);

    auto graph = std::make_unique<liz::PipelineGraph>("Demo Pipeline");
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
    executor.execute("Demo Pipeline");

    {
        std::ostringstream oss;
        oss << "  Pipelines: " << executor.pipeline_count();
        LIZ_INFO(oss.str());
    }

    std::cout << std::endl;

    // ═════════════════════════════════════════════════════════════════════
    // 7. Save project
    // ═════════════════════════════════════════════════════════════════════
    LIZ_INFO("[7] Saving project...");
    proj_mgr.save_project(project->uuid());
    {
        std::ostringstream oss;
        oss << "  State: " << liz::project_state_to_string(project->state());
        LIZ_INFO(oss.str());
    }

    std::cout << std::endl;

    // ═════════════════════════════════════════════════════════════════════
    // 8. Close project
    // ═════════════════════════════════════════════════════════════════════
    LIZ_INFO("[8] Closing project...");
    proj_mgr.close_project(project->uuid());
    {
        std::ostringstream oss;
        oss << "  State: " << liz::project_state_to_string(project->state());
        LIZ_INFO(oss.str());
    }

    std::cout << std::endl;

    // ═════════════════════════════════════════════════════════════════════
    // 9. Show statistics
    // ═════════════════════════════════════════════════════════════════════
    LIZ_INFO("[9] Project system statistics:");
    {
        auto stats = proj_mgr.statistics();
        std::ostringstream oss;
        oss << "  " << stats.to_string();
        LIZ_INFO(oss.str());
    }

    std::cout << std::endl;

    // ═════════════════════════════════════════════════════════════════════
    // 10. Show events published
    // ═════════════════════════════════════════════════════════════════════
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

    // ═════════════════════════════════════════════════════════════════════
    // 11. Diagnostics report with project data
    // ═════════════════════════════════════════════════════════════════════
    LIZ_INFO("[11] Diagnostics report (updated):");
    {
        liz::DiagnosticsDataProvider provider;
        provider.events_published = bus.event_count();
        provider.services_active = 0;
        provider.assets_active = asset_mgr.statistics().active_assets;
        provider.pipelines_active = executor.pipeline_count();
        provider.pipeline_nodes = 5;

        auto proj_stats = proj_mgr.statistics();
        provider.active_project = proj_stats.active_project;
        provider.open_projects  = proj_stats.projects_open;
        provider.saved_projects = proj_stats.projects_saved;

        liz::DiagnosticsManager diag;
        diag.set_provider(&provider);
        diag.capture_snapshot("after_sprint17");

        auto engine_stats = diag.statistics();
        std::ostringstream oss;
        oss << "  " << engine_stats.to_string();
        LIZ_INFO(oss.str());

        diag.print_report();
    }

    std::cout << std::endl;

    // ═════════════════════════════════════════════════════════════════════
    // 12. Service Registry integration
    // ═════════════════════════════════════════════════════════════════════
    LIZ_INFO("[12] Service Registry integration:");
    {
        liz::ServiceRegistry registry;
        registry.register_service("ProjectManager", liz::ServiceType::ProjectManager, "1.0.0");
        registry.log_statistics();
        registry.clear();
    }

    std::cout << std::endl;

    // ═════════════════════════════════════════════════════════════════════
    // 13. Cleanup
    // ═════════════════════════════════════════════════════════════════════
    executor.clear();
    executor.shutdown();
    asset_mgr.shutdown();
    proj_mgr.clear();
    proj_mgr.shutdown();
    bus.unsubscribe(&listener);

    LIZ_INFO("Project System demo completed");
    return 0;
}
