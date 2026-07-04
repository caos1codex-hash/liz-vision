#pragma once

#include <string>

namespace liz {

/// Types of services that can be registered in the Service Registry.
enum class ServiceType {
    Logger,
    Config,
    Runtime,
    ResourceManager,
    Scheduler,
    EventBus,
    GPU,
    Inference,
    Video,
    PluginManager,
    Performance,
    AssetManager,
    Unknown
};

/// Convert a ServiceType to a human-readable string.
inline const char* service_type_to_string(ServiceType type) {
    switch (type) {
        case ServiceType::Logger:          return "Logger";
        case ServiceType::Config:          return "Config";
        case ServiceType::Runtime:         return "Runtime";
        case ServiceType::ResourceManager: return "ResourceManager";
        case ServiceType::Scheduler:       return "Scheduler";
        case ServiceType::EventBus:        return "EventBus";
        case ServiceType::GPU:             return "GPU";
        case ServiceType::Inference:       return "Inference";
        case ServiceType::Video:           return "Video";
        case ServiceType::PluginManager:   return "PluginManager";
        case ServiceType::Performance:    return "Performance";
        case ServiceType::AssetManager:    return "AssetManager";
        case ServiceType::Unknown:         return "Unknown";
    }
    return "Unknown";
}

/// Lifecycle states of a service.
enum class ServiceState {
    Created,
    Initialized,
    Running,
    Stopped,
    Destroyed
};

/// Convert a ServiceState to a human-readable string.
inline const char* service_state_to_string(ServiceState state) {
    switch (state) {
        case ServiceState::Created:     return "Created";
        case ServiceState::Initialized: return "Initialized";
        case ServiceState::Running:     return "Running";
        case ServiceState::Stopped:     return "Stopped";
        case ServiceState::Destroyed:   return "Destroyed";
    }
    return "Unknown";
}

} // namespace liz
