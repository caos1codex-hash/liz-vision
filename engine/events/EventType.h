#pragma once

#include <string>

namespace liz {

/// Types of events that can be published through the EventBus.
///
/// New event types will be added in future sprints as more modules
/// adopt event-driven communication.
enum class EventType {
    EngineStarted,
    EngineStopped,
    RuntimeInitialized,
    ResourceCreated,
    ResourceDestroyed,
    FrameDecoded,
    TensorCreated,
    BatchCreated,
    InferenceStarted,
    InferenceFinished,
    GPUCommandSubmitted,
    GPUCommandCompleted,
    PluginLoading,
    PluginLoaded,
    PluginUnloading,
    PluginUnloaded,
    PluginReloaded,
    PluginLoadFailed,
    ExportStarted,
    ExportFinished,
    AssetCreated,
    AssetLoaded,
    AssetUnloaded,
    AssetDestroyed,
    PipelineCreated,
    PipelineDestroyed,
    NodeCreated,
    NodeExecuted,
    PipelineFinished,
    ProjectCreated,
    ProjectOpened,
    ProjectSaved,
    ProjectClosed,
    ProjectDestroyed,
    WorkspaceCreated,
    WorkspaceOpened,
    WorkspaceActivated,
    WorkspaceClosed,
    WorkspaceDestroyed,
    ProjectAddedToWorkspace,
    ProjectRemovedFromWorkspace,
    CloudSyncStarted,
    CloudSyncCompleted,
    CloudSyncFailed,
    CloudConflictDetected,
    JobCreated,
    JobQueued,
    JobStarted,
    JobCompleted,
    JobFailed,
    JobCancelled,
    Custom
};

/// Convert an EventType to a human-readable string.
inline const char* event_type_to_string(EventType type) {
    switch (type) {
        case EventType::EngineStarted:         return "EngineStarted";
        case EventType::EngineStopped:         return "EngineStopped";
        case EventType::RuntimeInitialized:    return "RuntimeInitialized";
        case EventType::ResourceCreated:       return "ResourceCreated";
        case EventType::ResourceDestroyed:     return "ResourceDestroyed";
        case EventType::FrameDecoded:          return "FrameDecoded";
        case EventType::TensorCreated:         return "TensorCreated";
        case EventType::BatchCreated:          return "BatchCreated";
        case EventType::InferenceStarted:      return "InferenceStarted";
        case EventType::InferenceFinished:     return "InferenceFinished";
        case EventType::GPUCommandSubmitted:   return "GPUCommandSubmitted";
        case EventType::GPUCommandCompleted:   return "GPUCommandCompleted";
        case EventType::PluginLoading:       return "PluginLoading";
        case EventType::PluginLoaded:        return "PluginLoaded";
        case EventType::PluginUnloading:     return "PluginUnloading";
        case EventType::PluginUnloaded:       return "PluginUnloaded";
        case EventType::PluginReloaded:      return "PluginReloaded";
        case EventType::PluginLoadFailed:     return "PluginLoadFailed";
        case EventType::ExportStarted:         return "ExportStarted";
        case EventType::ExportFinished:        return "ExportFinished";
        case EventType::AssetCreated:          return "AssetCreated";
        case EventType::AssetLoaded:           return "AssetLoaded";
        case EventType::AssetUnloaded:         return "AssetUnloaded";
        case EventType::AssetDestroyed:        return "AssetDestroyed";
        case EventType::PipelineCreated:       return "PipelineCreated";
        case EventType::PipelineDestroyed:     return "PipelineDestroyed";
        case EventType::NodeCreated:           return "NodeCreated";
        case EventType::NodeExecuted:          return "NodeExecuted";
        case EventType::PipelineFinished:      return "PipelineFinished";
        case EventType::ProjectCreated:        return "ProjectCreated";
        case EventType::ProjectOpened:          return "ProjectOpened";
        case EventType::ProjectSaved:            return "ProjectSaved";
        case EventType::ProjectClosed:          return "ProjectClosed";
        case EventType::ProjectDestroyed:        return "ProjectDestroyed";
        case EventType::WorkspaceCreated:        return "WorkspaceCreated";
        case EventType::WorkspaceOpened:          return "WorkspaceOpened";
        case EventType::WorkspaceActivated:       return "WorkspaceActivated";
        case EventType::WorkspaceClosed:          return "WorkspaceClosed";
        case EventType::WorkspaceDestroyed:        return "WorkspaceDestroyed";
        case EventType::ProjectAddedToWorkspace:  return "ProjectAddedToWorkspace";
        case EventType::ProjectRemovedFromWorkspace: return "ProjectRemovedFromWorkspace";
        case EventType::CloudSyncStarted:        return "CloudSyncStarted";
        case EventType::CloudSyncCompleted:       return "CloudSyncCompleted";
        case EventType::CloudSyncFailed:          return "CloudSyncFailed";
        case EventType::CloudConflictDetected:     return "CloudConflictDetected";
        case EventType::JobCreated:             return "JobCreated";
        case EventType::JobQueued:              return "JobQueued";
        case EventType::JobStarted:             return "JobStarted";
        case EventType::JobCompleted:           return "JobCompleted";
        case EventType::JobFailed:              return "JobFailed";
        case EventType::JobCancelled:            return "JobCancelled";
        case EventType::Custom:                return "Custom";
    }
    return "Unknown";
}

/// Event priority levels.  Higher priority events may be delivered first
/// in future implementations (currently all delivered in FIFO order).
enum class EventPriority {
    Low,
    Normal,
    High,
    Critical
};

/// Convert an EventPriority to a human-readable string.
inline const char* event_priority_to_string(EventPriority priority) {
    switch (priority) {
        case EventPriority::Low:      return "Low";
        case EventPriority::Normal:   return "Normal";
        case EventPriority::High:     return "High";
        case EventPriority::Critical: return "Critical";
    }
    return "Unknown";
}

} // namespace liz