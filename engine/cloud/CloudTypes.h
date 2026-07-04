#pragma once

#include <string>

namespace liz {

/// Sync states for cloud sync items.
enum class CloudSyncState {
    Pending,
    Syncing,
    Synced,
    Conflict,
    Failed
};

/// Convert a CloudSyncState to a human-readable string.
inline const char* cloud_sync_state_to_string(CloudSyncState state) {
    switch (state) {
        case CloudSyncState::Pending:  return "Pending";
        case CloudSyncState::Syncing: return "Syncing";
        case CloudSyncState::Synced:  return "Synced";
        case CloudSyncState::Conflict: return "Conflict";
        case CloudSyncState::Failed:  return "Failed";
    }
    return "Unknown";
}

/// Types of items that can be cloud-synced.
enum class CloudSyncType {
    Project,
    Workspace,
    Asset,
    Pipeline,
    Settings
};

/// Convert a CloudSyncType to a human-readable string.
inline const char* cloud_sync_type_to_string(CloudSyncType type) {
    switch (type) {
        case CloudSyncType::Project:   return "Project";
        case CloudSyncType::Workspace: return "Workspace";
        case CloudSyncType::Asset:     return "Asset";
        case CloudSyncType::Pipeline:  return "Pipeline";
        case CloudSyncType::Settings:  return "Settings";
    }
    return "Unknown";
}

} // namespace liz
