#pragma once

#include <string>

namespace liz {

/// Semantic category of a managed asset.
enum class AssetType {
    Unknown,
    Video,
    Image,
    Audio,
    Model,
    Tensor,
    Shader,
    Plugin,
    Configuration,
    Project,
    Temporary
};

/// Convert an AssetType to a human-readable string.
inline const char* asset_type_to_string(AssetType type) {
    switch (type) {
        case AssetType::Unknown:        return "Unknown";
        case AssetType::Video:         return "Video";
        case AssetType::Image:          return "Image";
        case AssetType::Audio:          return "Audio";
        case AssetType::Model:          return "Model";
        case AssetType::Tensor:         return "Tensor";
        case AssetType::Shader:         return "Shader";
        case AssetType::Plugin:         return "Plugin";
        case AssetType::Configuration:  return "Configuration";
        case AssetType::Project:        return "Project";
        case AssetType::Temporary:      return "Temporary";
    }
    return "Unknown";
}

/// Lifecycle state of an asset.
enum class AssetState {
    Unloaded,
    Loading,
    Loaded,
    Unloading,
    Error
};

/// Convert an AssetState to a human-readable string.
inline const char* asset_state_to_string(AssetState state) {
    switch (state) {
        case AssetState::Unloaded:  return "Unloaded";
        case AssetState::Loading:   return "Loading";
        case AssetState::Loaded:    return "Loaded";
        case AssetState::Unloading: return "Unloading";
        case AssetState::Error:     return "Error";
    }
    return "Unknown";
}

} // namespace liz
