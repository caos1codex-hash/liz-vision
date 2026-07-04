#pragma once

#include <string>

namespace liz {

/// Lifecycle states of a Workspace.
enum class WorkspaceState {
    Created,
    Opened,
    Active,
    Closed
};

/// Convert a WorkspaceState to a human-readable string.
inline const char* workspace_state_to_string(WorkspaceState state) {
    switch (state) {
        case WorkspaceState::Created: return "Created";
        case WorkspaceState::Opened:  return "Opened";
        case WorkspaceState::Active:  return "Active";
        case WorkspaceState::Closed:  return "Closed";
    }
    return "Unknown";
}

} // namespace liz
