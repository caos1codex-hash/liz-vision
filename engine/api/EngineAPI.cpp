#include "engine/api/EngineAPI.h"
#include "engine/core/Engine.h"
#include "engine/core/Logger.h"
#include "engine/core/Config.h"
#include "engine/core/EngineContext.h"
#include "engine/services/ServiceRegistry.h"
#include "engine/services/ServiceType.h"
#include "engine/services/Service.h"
#include "engine/diagnostics/DiagnosticsManager.h"
#include "engine/diagnostics/PerformanceSnapshot.h"
#include "engine/diagnostics/EngineStatistics.h"
#include "engine/assets/AssetManager.h"
#include "engine/assets/AssetType.h"
#include "engine/events/EventBus.h"
#include "engine/pipeline/PipelineExecutor.h"
#include "engine/pipeline/PipelineGraph.h"
#include "engine/project/ProjectManager.h"
#include "engine/project/Project.h"
#include "engine/project/ProjectTypes.h"
#include "engine/workspace/WorkspaceManager.h"
#include "engine/workspace/Workspace.h"
#include "engine/workspace/WorkspaceTypes.h"

#include <algorithm>
#include <chrono>
#include <sstream>

namespace liz {

// ── Current API version ─────────────────────────────────────────────────

static constexpr ApiVersion kCurrentApiVersion{1, 0, 0, 1};

// ── Constructor / Destructor ──────────────────────────────────────────

EngineAPI::EngineAPI() = default;

EngineAPI::~EngineAPI() {
    if (initialized_) {
        shutdown();
    }
}

// ── Lifecycle ─────────────────────────────────────────────────────────

ApiResult EngineAPI::initialize(const EngineBuilder& config) {
    if (initialized_) {
        LIZ_WARN("EngineAPI: already initialized");
        return ApiResult::Warning;
    }

    if (!config.is_built()) {
        LIZ_ERROR("EngineAPI: cannot initialize — EngineBuilder::build() was not called");
        return ApiResult::InvalidArgument;
    }

    config_ = config;

    LIZ_INFO("=== LIZ EngineAPI initializing ===");

    // 1. Create and initialize the internal Engine.
    engine_ = std::make_unique<Engine>();
    if (!engine_->initialize()) {
        LIZ_ERROR("EngineAPI: internal engine initialization failed");
        engine_.reset();
        return ApiResult::InternalError;
    }

    // 2. Create Service Registry and register core services.
    service_registry_ = std::make_unique<ServiceRegistry>();
    service_registry_->register_service("Logger",          ServiceType::Logger, "1.0.0");
    service_registry_->register_service("Config",          ServiceType::Config, "1.0.0");
    service_registry_->register_service("Runtime",        ServiceType::Runtime, "1.0.0");
    service_registry_->register_service("Scheduler",       ServiceType::Scheduler, "1.0.0");
    service_registry_->register_service("EventBus",        ServiceType::EventBus, "1.0.0");
    service_registry_->register_service("GPU",             ServiceType::GPU, "1.0.0");
    service_registry_->register_service("Inference",       ServiceType::Inference, "1.0.0");
    service_registry_->register_service("Video",           ServiceType::Video, "1.0.0");
    service_registry_->register_service("Performance",    ServiceType::Performance, "1.0.0");

    if (config_.plugins_enabled()) {
        service_registry_->register_service("PluginManager", ServiceType::PluginManager, "1.0.0");
    }

    // 3. Create EventBus if enabled.
    if (config_.events_enabled()) {
        event_bus_ = std::make_unique<EventBus>();
    }

    // 4. Create DiagnosticsManager if enabled.
    if (config_.diagnostics_enabled()) {
        diagnostics_ = std::make_unique<DiagnosticsManager>();
        data_provider_ = std::make_unique<DiagnosticsDataProvider>();
        diagnostics_->set_provider(data_provider_.get());

        service_registry_->register_service("Diagnostics", ServiceType::Unknown, "1.0.0");
    }

    // 5. Create AssetManager if enabled.
    if (config_.assets_enabled()) {
        asset_manager_ = std::make_unique<AssetManager>();
        asset_manager_->initialize();

        if (event_bus_) {
            asset_manager_->set_event_bus(event_bus_.get());
        }

        service_registry_->register_service("AssetManager", ServiceType::AssetManager, "1.0.0");
    }

// 6. Create PipelineExecutor.
    pipeline_executor_ = std::make_unique<PipelineExecutor>();
    pipeline_executor_->initialize();

    if (event_bus_) {
        pipeline_executor_->set_event_bus(event_bus_.get());
    }

    service_registry_->register_service("PipelineExecutor", ServiceType::PipelineExecutor, "1.0.0");

    // 7. Create ProjectManager.
    project_manager_ = std::make_unique<ProjectManager>();
    project_manager_->initialize();

    if (event_bus_) {
        project_manager_->set_event_bus(event_bus_.get());
    }

    service_registry_->register_service("ProjectManager", ServiceType::ProjectManager, "1.0.0");

    // 8. Create WorkspaceManager.
    workspace_manager_ = std::make_unique<WorkspaceManager>();
    workspace_manager_->initialize();

    if (event_bus_) {
        workspace_manager_->set_event_bus(event_bus_.get());
    }

    service_registry_->register_service("WorkspaceManager", ServiceType::WorkspaceManager, "1.0.0");

    // 9. Create an implicit default session.
    SessionInfo default_session;
    create_session(default_session);

    initialized_ = true;

    LIZ_INFO("=== LIZ EngineAPI ready ===");
    return ApiResult::Success;
}

ApiResult EngineAPI::shutdown() {
    if (!initialized_) {
        LIZ_WARN("EngineAPI: shutdown called but not initialized");
        return ApiResult::Warning;
    }

    LIZ_INFO("=== LIZ EngineAPI shutting down ===");

    // Destroy all sessions.
    sessions_.clear();

    // Shutdown subsystems in reverse order.
    if (project_manager_) {
        project_manager_->shutdown();
        project_manager_.reset();
    }

    if (workspace_manager_) {
        workspace_manager_->shutdown();
        workspace_manager_.reset();
    }

    if (pipeline_executor_) {
        pipeline_executor_->shutdown();
        pipeline_executor_.reset();
    }

    if (asset_manager_) {
        asset_manager_->shutdown();
        asset_manager_.reset();
    }

    if (diagnostics_) {
        diagnostics_->reset();
        diagnostics_.reset();
        data_provider_.reset();
    }

    event_bus_.reset();

    if (service_registry_) {
        service_registry_->clear();
        service_registry_.reset();
    }

    if (engine_) {
        engine_->shutdown();
        engine_.reset();
    }

    initialized_ = false;

    LIZ_INFO("=== LIZ EngineAPI stopped ===");
    return ApiResult::Success;
}

bool EngineAPI::is_initialized() const {
    return initialized_;
}

// ── Sessions ─────────────────────────────────────────────────────────

ApiResult EngineAPI::create_session(SessionInfo& out_info) {
    auto session = std::make_unique<EngineSession>();
    out_info.session_id = session->id();
    out_info.state       = session->is_running() ? "Running" : "Stopped";
    out_info.uptime_ms   = 0.0;

    std::ostringstream oss;
    oss << "EngineAPI: session created — id=" << out_info.session_id;
    LIZ_INFO(oss.str());

    sessions_.push_back(std::move(session));
    return ApiResult::Success;
}

ApiResult EngineAPI::destroy_session(const std::string& session_id) {
    auto it = std::find_if(sessions_.begin(), sessions_.end(),
        [&session_id](const auto& s) { return s->id() == session_id; });

    if (it == sessions_.end()) {
        return ApiResult::NotFound;
    }

    (*it)->shutdown();
    sessions_.erase(it);

    std::ostringstream oss;
    oss << "EngineAPI: session destroyed — id=" << session_id;
    LIZ_INFO(oss.str());

    return ApiResult::Success;
}

std::vector<SessionInfo> EngineAPI::list_sessions() const {
    std::vector<SessionInfo> result;
    for (const auto& s : sessions_) {
        SessionInfo info;
        info.session_id = s->id();
        info.state      = s->is_running() ? "Running" : "Stopped";
        info.uptime_ms  = s->active_time_ms();
        result.push_back(info);
    }
    return result;
}

// ── Version / Info ────────────────────────────────────────────────────

ApiVersion EngineAPI::version() const {
    return kCurrentApiVersion;
}

EngineInfo EngineAPI::engine_info() const {
    EngineInfo info;
    info.session_id          = engine_ ? engine_->session_id() : "";
    info.state                = engine_ ? engine_state_to_string(engine_->context().state()) : "Unknown";
    info.application_name     = config_.application_name();
    info.application_version  = config_.application_version();
    info.engine_version      = kCurrentApiVersion.version_string();
    info.gpu_enabled          = config_.gpu_enabled();
    info.diagnostics_enabled  = config_.diagnostics_enabled();
    info.assets_enabled      = config_.assets_enabled();
    info.events_enabled      = config_.events_enabled();
    return info;
}

// ── Statistics ─────────────────────────────────────────────────────────

ApiStatistics EngineAPI::statistics() const {
    ApiStatistics stats;

    // Uptime from engine context state.
    if (initialized_) {
        stats.uptime_ms = 0.0; // Will be updated by actual runtime tracking.
    }

    // Service count.
    if (service_registry_) {
        stats.services_registered = service_registry_->count();
    }

    // Asset count.
    if (asset_manager_) {
        auto asset_stats = asset_manager_->statistics();
        stats.assets_active = asset_stats.active_assets;
    }

    return stats;
}

// ── Service info ──────────────────────────────────────────────────────

ServiceInfoList EngineAPI::service_info() const {
    ServiceInfoList result;

    if (!service_registry_) return result;

    auto names = service_registry_->list();
    for (const auto& name : names) {
        auto* svc = service_registry_->find(name);
        if (svc) {
            ServiceInfo info;
            info.name    = svc->name();
            info.type    = service_type_to_string(svc->type());
            info.state   = service_state_to_string(svc->state());
            info.version = svc->version();
            result.push_back(info);
        }
    }

    return result;
}

// ── Asset info ────────────────────────────────────────────────────────

AssetInfoList EngineAPI::asset_info() const {
    AssetInfoList result;

    if (!asset_manager_) return result;

    auto names = asset_manager_->list();
    for (const auto& name : names) {
        auto* asset = asset_manager_->find_by_name(name);
        if (asset) {
            AssetInfo info;
            info.name       = asset->name();
            info.type       = asset_type_to_string(asset->type());
            info.state      = asset_state_to_string(asset->state());
            info.version    = asset->version();
            info.size_bytes = asset->size_bytes();
            result.push_back(info);
        }
    }

    return result;
}

// ── Diagnostics info ───────────────────────────────────────────────────

DiagnosticsInfo EngineAPI::diagnostics_info() const {
    DiagnosticsInfo info;

    if (!diagnostics_) return info;

    info.snapshot_count = diagnostics_->snapshot_count();

    // Get statistics summary.
    auto stats = diagnostics_->statistics();
    info.statistics_summary = stats.to_string();

    // Get latest performance snapshot summary.
    const auto& snapshots = diagnostics_->snapshots();
    if (!snapshots.empty()) {
        info.performance_summary = snapshots.back().to_string();
    } else {
        info.performance_summary = "(no snapshots)";
    }

    return info;
}

// ── Pipeline info ────────────────────────────────────────────────────

ApiResult EngineAPI::create_pipeline(const std::string& name) {
    if (!initialized_) return ApiResult::Failed;

    auto graph = std::make_unique<PipelineGraph>(name);
    if (event_bus_) graph->set_event_bus(event_bus_.get());

    if (!pipeline_executor_->register_pipeline(std::move(graph))) {
        return ApiResult::AlreadyExists;
    }

    std::ostringstream oss;
    oss << "EngineAPI: pipeline created — name=" << name;
    LIZ_INFO(oss.str());

    return ApiResult::Success;
}

ApiResult EngineAPI::destroy_pipeline(const std::string& name) {
    if (!initialized_) return ApiResult::Failed;

    if (!pipeline_executor_->destroy_pipeline(name)) {
        return ApiResult::NotFound;
    }

    std::ostringstream oss;
    oss << "EngineAPI: pipeline destroyed — name=" << name;
    LIZ_INFO(oss.str());

    return ApiResult::Success;
}

PipelineStatistics EngineAPI::pipeline_statistics(const std::string& name) const {
    if (!pipeline_executor_) return PipelineStatistics{};
    return pipeline_executor_->pipeline_statistics(name);
}

PipelineInfoList EngineAPI::list_pipelines() const {
    PipelineInfoList result;
    if (!pipeline_executor_) return result;

    auto names = pipeline_executor_->list_pipelines();
    for (const auto& name : names) {
        auto stats = pipeline_executor_->pipeline_statistics(name);
        PipelineInfo info;
        info.name = name;
        info.nodes = stats.total_nodes;
        info.edges = stats.total_edges;
        info.executed = stats.nodes_executed;
        info.failed = stats.nodes_failed;
        info.disabled = stats.nodes_disabled;
        info.execution_time_ms = stats.execution_time_ms;
        info.valid = stats.validation_result;
        result.push_back(info);
    }
    return result;
}

// ── Project info ──────────────────────────────────────────────────

ApiResult EngineAPI::create_project(const std::string& name, ProjectInfo& out_info) {
    if (!initialized_) return ApiResult::Failed;

    auto* proj = project_manager_->create_project(name);
    if (!proj) {
        return ApiResult::AlreadyExists;
    }

    out_info.uuid    = proj->uuid();
    out_info.name    = proj->name();
    out_info.state   = project_state_to_string(proj->state());
    out_info.author  = proj->author();
    out_info.version = proj->version();

    std::ostringstream oss;
    oss << "EngineAPI: project created — name=" << name;
    LIZ_INFO(oss.str());

    return ApiResult::Success;
}

ApiResult EngineAPI::open_project(const std::string& uuid) {
    if (!initialized_) return ApiResult::Failed;

    if (!project_manager_->open_project(uuid)) {
        return ApiResult::NotFound;
    }

    std::ostringstream oss;
    oss << "EngineAPI: project opened — uuid=" << uuid;
    LIZ_INFO(oss.str());

    return ApiResult::Success;
}

ApiResult EngineAPI::save_project(const std::string& uuid) {
    if (!initialized_) return ApiResult::Failed;

    if (!project_manager_->save_project(uuid)) {
        return ApiResult::NotFound;
    }

    std::ostringstream oss;
    oss << "EngineAPI: project saved — uuid=" << uuid;
    LIZ_INFO(oss.str());

    return ApiResult::Success;
}

ApiResult EngineAPI::close_project(const std::string& uuid) {
    if (!initialized_) return ApiResult::Failed;

    if (!project_manager_->close_project(uuid)) {
        return ApiResult::NotFound;
    }

    std::ostringstream oss;
    oss << "EngineAPI: project closed — uuid=" << uuid;
    LIZ_INFO(oss.str());

    return ApiResult::Success;
}

ApiResult EngineAPI::destroy_project(const std::string& uuid) {
    if (!initialized_) return ApiResult::Failed;

    if (!project_manager_->destroy_project(uuid)) {
        return ApiResult::NotFound;
    }

    std::ostringstream oss;
    oss << "EngineAPI: project destroyed — uuid=" << uuid;
    LIZ_INFO(oss.str());

    return ApiResult::Success;
}

ApiResult EngineAPI::current_project(ProjectInfo& out_info) const {
    if (!project_manager_) return ApiResult::Failed;

    auto* proj = project_manager_->current_project();
    if (!proj) {
        return ApiResult::NotFound;
    }

    out_info.uuid    = proj->uuid();
    out_info.name    = proj->name();
    out_info.state   = project_state_to_string(proj->state());
    out_info.author  = proj->author();
    out_info.version = proj->version();

    return ApiResult::Success;
}

ApiProjectStatistics EngineAPI::project_statistics() const {
    ApiProjectStatistics stats;

    if (!project_manager_) return stats;

    auto proj_stats = project_manager_->statistics();
    stats.projects_created = proj_stats.projects_created;
    stats.projects_open    = proj_stats.projects_open;
    stats.projects_saved  = proj_stats.projects_saved;
    stats.projects_closed = proj_stats.projects_closed;
    stats.active_project   = proj_stats.active_project;
    stats.assets          = proj_stats.assets;
    stats.pipelines       = proj_stats.pipelines;
    stats.services        = proj_stats.services;
    stats.runtime_seconds = proj_stats.runtime_seconds;

    // Populate from existing subsystems.
    if (asset_manager_) {
        auto asset_stats = asset_manager_->statistics();
        stats.assets = asset_stats.active_assets;
    }
    if (pipeline_executor_) {
        stats.pipelines = pipeline_executor_->pipeline_count();
    }
    if (service_registry_) {
        stats.services = service_registry_->count();
    }

    return stats;
}

ProjectInfoList EngineAPI::list_projects() const {
    ProjectInfoList result;
    if (!project_manager_) return result;

    auto names = project_manager_->list_projects();
    for (const auto& name : names) {
        auto* proj = project_manager_->find_project_by_name(name);
        if (proj) {
            ProjectInfo info;
            info.uuid    = proj->uuid();
            info.name    = proj->name();
            info.state   = project_state_to_string(proj->state());
            info.author  = proj->author();
            info.version = proj->version();
            result.push_back(info);
        }
    }
    return result;
}

// ── Workspace info ──────────────────────────────────────────────────

ApiResult EngineAPI::create_workspace(const std::string& name, WorkspaceInfo& out_info) {
    if (!initialized_) return ApiResult::Failed;

    auto* ws = workspace_manager_->create_workspace(name);
    if (!ws) {
        return ApiResult::AlreadyExists;
    }

    out_info.uuid     = ws->uuid();
    out_info.name     = ws->name();
    out_info.state    = workspace_state_to_string(ws->state());
    out_info.projects = ws->project_count();

    std::ostringstream oss;
    oss << "EngineAPI: workspace created — name=" << name;
    LIZ_INFO(oss.str());

    return ApiResult::Success;
}

ApiResult EngineAPI::open_workspace(const std::string& uuid) {
    if (!initialized_) return ApiResult::Failed;

    if (!workspace_manager_->open_workspace(uuid)) {
        return ApiResult::NotFound;
    }

    std::ostringstream oss;
    oss << "EngineAPI: workspace opened — uuid=" << uuid;
    LIZ_INFO(oss.str());

    return ApiResult::Success;
}

ApiResult EngineAPI::close_workspace(const std::string& uuid) {
    if (!initialized_) return ApiResult::Failed;

    if (!workspace_manager_->close_workspace(uuid)) {
        return ApiResult::NotFound;
    }

    std::ostringstream oss;
    oss << "EngineAPI: workspace closed — uuid=" << uuid;
    LIZ_INFO(oss.str());

    return ApiResult::Success;
}

ApiResult EngineAPI::destroy_workspace(const std::string& uuid) {
    if (!initialized_) return ApiResult::Failed;

    if (!workspace_manager_->destroy_workspace(uuid)) {
        return ApiResult::NotFound;
    }

    std::ostringstream oss;
    oss << "EngineAPI: workspace destroyed — uuid=" << uuid;
    LIZ_INFO(oss.str());

    return ApiResult::Success;
}

ApiResult EngineAPI::set_active_workspace(const std::string& uuid) {
    if (!initialized_) return ApiResult::Failed;

    if (!workspace_manager_->set_active_workspace(uuid)) {
        return ApiResult::NotFound;
    }

    std::ostringstream oss;
    oss << "EngineAPI: active workspace set — uuid=" << uuid;
    LIZ_INFO(oss.str());

    return ApiResult::Success;
}

ApiResult EngineAPI::active_workspace(WorkspaceInfo& out_info) const {
    if (!workspace_manager_) return ApiResult::Failed;

    auto* ws = workspace_manager_->active_workspace();
    if (!ws) {
        return ApiResult::NotFound;
    }

    out_info.uuid     = ws->uuid();
    out_info.name     = ws->name();
    out_info.state    = workspace_state_to_string(ws->state());
    out_info.projects = ws->project_count();

    return ApiResult::Success;
}

ApiWorkspaceStatistics EngineAPI::workspace_statistics() const {
    ApiWorkspaceStatistics stats;

    if (!workspace_manager_) return stats;

    auto ws_stats = workspace_manager_->statistics();
    stats.workspaces_created = ws_stats.workspaces_created;
    stats.workspaces_open    = ws_stats.workspaces_open;
    stats.active_workspace   = ws_stats.active_workspace;
    stats.projects_loaded   = ws_stats.projects_loaded;
    stats.assets_loaded     = ws_stats.assets_loaded;
    stats.pipelines_loaded  = ws_stats.pipelines_loaded;
    stats.runtime_seconds   = ws_stats.runtime_seconds;

    // Populate from existing subsystems.
    if (asset_manager_) {
        auto asset_stats = asset_manager_->statistics();
        stats.assets_loaded = asset_stats.active_assets;
    }
    if (pipeline_executor_) {
        stats.pipelines_loaded = pipeline_executor_->pipeline_count();
    }

    return stats;
}

WorkspaceInfoList EngineAPI::list_workspaces() const {
    WorkspaceInfoList result;
    if (!workspace_manager_) return result;

    auto names = workspace_manager_->list_workspaces();
    for (const auto& name : names) {
        auto* ws = workspace_manager_->find_workspace_by_name(name);
        if (ws) {
            WorkspaceInfo info;
            info.uuid     = ws->uuid();
            info.name     = ws->name();
            info.state    = workspace_state_to_string(ws->state());
            info.projects = ws->project_count();
            result.push_back(info);
        }
    }
    return result;
}

ApiResult EngineAPI::add_project_to_workspace(const std::string& workspace_uuid, const std::string& project_uuid) {
    if (!initialized_) return ApiResult::Failed;

    if (!workspace_manager_->add_project(workspace_uuid, project_uuid)) {
        return ApiResult::NotFound;
    }

    std::ostringstream oss;
    oss << "EngineAPI: project added to workspace";
    LIZ_INFO(oss.str());

    return ApiResult::Success;
}

ApiResult EngineAPI::remove_project_from_workspace(const std::string& workspace_uuid, const std::string& project_uuid) {
    if (!initialized_) return ApiResult::Failed;

    if (!workspace_manager_->remove_project(workspace_uuid, project_uuid)) {
        return ApiResult::NotFound;
    }

    std::ostringstream oss;
    oss << "EngineAPI: project removed from workspace";
    LIZ_INFO(oss.str());

    return ApiResult::Success;
}

} // namespace liz
