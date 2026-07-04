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
    PluginLoaded,
    PluginUnloaded,
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
        case EventType::PluginLoaded:          return "PluginLoaded";
        case EventType::PluginUnloaded:        return "PluginUnloaded";
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