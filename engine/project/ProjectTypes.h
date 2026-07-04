#pragma once

#include <string>

namespace liz {

/// Lifecycle states of a Project.
enum class ProjectState {
    Created,
    Opened,
    Modified,
    Saved,
    Closed
};

/// Convert a ProjectState to a human-readable string.
inline const char* project_state_to_string(ProjectState state) {
    switch (state) {
        case ProjectState::Created:  return "Created";
        case ProjectState::Opened:   return "Opened";
        case ProjectState::Modified: return "Modified";
        case ProjectState::Saved:    return "Saved";
        case ProjectState::Closed:   return "Closed";
    }
    return "Unknown";
}

} // namespace liz
