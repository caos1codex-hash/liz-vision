#pragma once

#include "engine/api/ApiVersion.h"
#include "engine/api/ApiResult.h"
#include "engine/api/ApiTypes.h"
#include "engine/api/EngineBuilder.h"
#include "engine/api/EngineSession.h"
#include "engine/pipeline/PipelineStatistics.h"

#include <memory>
#include <string>
#include <vector>

namespace liz {

// Forward declarations — internal engine components are hidden.
class Engine;
class ServiceRegistry;
class DiagnosticsManager;
class AssetManager;
class EventBus;
class DiagnosticsDataProvider;
class PipelineExecutor;
class PipelineGraph;
class ProjectManager;

/// Public API facade for the LIZ Engine.
///
/// This is the ONLY interface that external applications should use.
/// All internal subsystems (Logger, Scheduler, GPU, Inference, etc.)
/// are hidden behind this facade.
///
/// Usage:
///   EngineBuilder builder;
///   builder.set_application_name("MyApp").enable_gpu(true).build();
///
///   EngineAPI api;
///   api.initialize(builder);
///
///   auto session = api.create_session();
///   auto stats   = api.statistics();
///
///   api.destroy_session(session.id());
///   api.shutdown();
class EngineAPI {
public:
    EngineAPI();
    ~EngineAPI();

    // Non-copyable.
    EngineAPI(const EngineAPI&) = delete;
    EngineAPI& operator=(const EngineAPI&) = delete;

    // ── Lifecycle ──────────────────────────────────────────────────────

    /// Initialize the engine using the configuration from EngineBuilder.
    ApiResult initialize(const EngineBuilder& config);

    /// Shut down the engine gracefully.
    ApiResult shutdown();

    /// Check if the engine is currently initialized and operational.
    bool is_initialized() const;

    // ── Sessions ───────────────────────────────────────────────────────

    /// Create a new public session.  Returns a SessionInfo with the ID.
    ApiResult create_session(SessionInfo& out_info);

    /// Destroy a session by its ID.
    ApiResult destroy_session(const std::string& session_id);

    /// Get information about all active sessions.
    std::vector<SessionInfo> list_sessions() const;

    // ── Version / Info ────────────────────────────────────────────────

    /// Get the API version.
    ApiVersion version() const;

    /// Get high-level engine information.
    EngineInfo engine_info() const;

    // ── Statistics ────────────────────────────────────────────────────

    /// Get engine statistics (simplified public view).
    ApiStatistics statistics() const;

    // ── Service info ──────────────────────────────────────────────────

    /// Get a list of all registered services (public view).
    ServiceInfoList service_info() const;

    // ── Asset info ────────────────────────────────────────────────────

    /// Get a list of all managed assets (public view).
    AssetInfoList asset_info() const;

    // ── Diagnostics info ─────────────────────────────────────────────

    /// Get diagnostics information (public view).
    DiagnosticsInfo diagnostics_info() const;

    // ── Pipeline info ───────────────────────────────────────────────

    /// Create a new pipeline graph by name.
    ApiResult create_pipeline(const std::string& name);

    /// Destroy a pipeline by name.
    ApiResult destroy_pipeline(const std::string& name);

    /// Get statistics for a specific pipeline.
    PipelineStatistics pipeline_statistics(const std::string& name) const;

    /// List all pipelines.
    PipelineInfoList list_pipelines() const;

    // ── Project info ───────────────────────────────────────────────

    /// Create a new project with the given name.
    ApiResult create_project(const std::string& name, ProjectInfo& out_info);

    /// Open a project by UUID.
    ApiResult open_project(const std::string& uuid);

    /// Save a project by UUID (simulated).
    ApiResult save_project(const std::string& uuid);

    /// Close a project by UUID.
    ApiResult close_project(const std::string& uuid);

    /// Destroy a project by UUID.
    ApiResult destroy_project(const std::string& uuid);

    /// Get the current active project info.  Returns NotFound if none.
    ApiResult current_project(ProjectInfo& out_info) const;

    /// Get project system statistics.
    ApiProjectStatistics project_statistics() const;

    /// List all projects.
    ProjectInfoList list_projects() const;

private:
    // Internal engine — never exposed.
    std::unique_ptr<Engine>                engine_;
    std::unique_ptr<ServiceRegistry>       service_registry_;
    std::unique_ptr<DiagnosticsManager>    diagnostics_;
    std::unique_ptr<AssetManager>          asset_manager_;
    std::unique_ptr<EventBus>               event_bus_;
    std::unique_ptr<DiagnosticsDataProvider> data_provider_;
    std::unique_ptr<PipelineExecutor>         pipeline_executor_;
    std::unique_ptr<ProjectManager>            project_manager_;

    // Public sessions.
    std::vector<std::unique_ptr<EngineSession>> sessions_;

    // Builder config snapshot.
    EngineBuilder config_;

    bool initialized_ = false;
};

} // namespace liz
