#pragma once

#include "engine/api/ApiVersion.h"
#include "engine/api/ApiResult.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace liz {

/// Information about the Public API itself.
struct ApiInfo {
    ApiVersion version;
    std::string build_date;
    std::string platform;
};

/// General engine statistics exposed through the public API.
///
/// This is a simplified view — internal details are NOT exposed.
struct ApiStatistics {
    // -- Timing --
    double uptime_ms = 0.0;

    // -- Counts --
    std::size_t services_registered = 0;
    std::size_t assets_active       = 0;
    std::size_t frames_processed   = 0;
    std::size_t events_published    = 0;

    // -- Memory --
    std::uint64_t ram_used  = 0;
    std::uint64_t vram_used = 0;

    /// Generate a formatted summary string.
    std::string to_string() const;
};

/// Engine information exposed through the public API.
///
/// Contains only what external applications need to know.
struct EngineInfo {
    std::string  session_id;
    std::string  state;
    std::string  application_name;
    std::string  application_version;
    std::string  engine_version;
    bool         gpu_enabled    = false;
    bool         diagnostics_enabled = false;
    bool         assets_enabled     = false;
    bool         events_enabled     = false;
};

/// Session information exposed through the public API.
struct SessionInfo {
    std::string session_id;
    std::string state;
    double      uptime_ms = 0.0;
};

/// Service information exposed through the public API.
struct ServiceInfo {
    std::string name;
    std::string type;
    std::string state;
    std::string version;
};

/// Asset information exposed through the public API.
struct AssetInfo {
    std::string name;
    std::string type;
    std::string state;
    std::string version;
    std::size_t size_bytes = 0;
};

/// Diagnostics information exposed through the public API.
struct DiagnosticsInfo {
    std::string performance_summary;
    std::string statistics_summary;
    std::size_t snapshot_count = 0;
};

/// Pipeline information exposed through the public API.
struct PipelineInfo {
    std::string name;
    std::size_t nodes = 0;
    std::size_t edges = 0;
    std::size_t executed = 0;
    std::size_t failed = 0;
    std::size_t disabled = 0;
    double     execution_time_ms = 0.0;
    bool       valid = true;
};

/// Project information exposed through the public API.
struct ProjectInfo {
    std::string uuid;
    std::string name;
    std::string state;
    std::string author;
    std::string version;
};

/// Project statistics exposed through the public API.
struct ApiProjectStatistics {
    std::size_t projects_created = 0;
    std::size_t projects_open     = 0;
    std::size_t projects_saved   = 0;
    std::size_t projects_closed  = 0;
    std::size_t active_project   = 0;
    std::size_t assets           = 0;
    std::size_t pipelines        = 0;
    std::size_t services          = 0;
    double     runtime_seconds   = 0.0;

    /// Generate a formatted summary string.
    std::string to_string() const;
};

/// Workspace information exposed through the public API.
struct WorkspaceInfo {
    std::string uuid;
    std::string name;
    std::string state;
    std::size_t projects = 0;
};

/// Workspace statistics exposed through the public API.
struct ApiWorkspaceStatistics {
    std::size_t workspaces_created = 0;
    std::size_t workspaces_open     = 0;
    std::size_t active_workspace   = 0;
    std::size_t projects_loaded    = 0;
    std::size_t assets_loaded      = 0;
    std::size_t pipelines_loaded   = 0;
    double     runtime_seconds     = 0.0;

    /// Generate a formatted summary string.
    std::string to_string() const;
};

/// Cloud sync item information exposed through the public API.
struct CloudSyncInfo {
    std::string uuid;
    std::string name;
    std::string type;
    std::string state;
    std::string error_message;
};

/// Cloud statistics exposed through the public API.
struct ApiCloudStatistics {
    std::size_t items_pending  = 0;
    std::size_t items_synced   = 0;
    std::size_t items_conflict = 0;
    std::size_t items_failed   = 0;
    double     total_sync_time_ms = 0.0;

    /// Generate a formatted summary string.
    std::string to_string() const;
};

/// Plugin information exposed through the public API.
struct PluginInfo {
    std::string uuid;
    std::string name;
    std::string author;
    std::string version;
    std::string category;
    std::string state;
    double     load_time_ms = 0.0;
};

/// Plugin loader statistics exposed through the public API.
struct ApiPluginStatistics {
    std::size_t plugins_registered = 0;
    std::size_t plugins_loaded     = 0;
    std::size_t plugins_failed     = 0;
    std::size_t plugins_reloaded   = 0;
    double     total_load_time_ms  = 0.0;

    /// Generate a formatted summary string.
    std::string to_string() const;
};

/// List of service/asset infos returned by query methods.
using ServiceInfoList = std::vector<ServiceInfo>;
using AssetInfoList   = std::vector<AssetInfo>;
using PipelineInfoList = std::vector<PipelineInfo>;
using ProjectInfoList = std::vector<ProjectInfo>;
using WorkspaceInfoList = std::vector<WorkspaceInfo>;
using CloudSyncInfoList = std::vector<CloudSyncInfo>;
using PluginInfoList = std::vector<PluginInfo>;

} // namespace liz
