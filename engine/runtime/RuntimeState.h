#pragma once

#include <string>
#include <string_view>

namespace liz {

/// High-level state of the LIZ Vision runtime.
///
/// These represent the broad lifecycle phases of the engine.
/// Transitions between states are validated by LifecycleManager.
enum class RuntimeState {
    Created,       // Runtime object instantiated, nothing initialized yet.
    Initializing,  // Subsystems are being brought up.
    Ready,         // All subsystems initialized, awaiting work.
    Running,       // Actively processing work.
    Paused,        // Processing suspended, can resume.
    Stopping,      // Graceful shutdown in progress.
    Stopped,       // Fully shut down, no further operations allowed.
    Error          // An unrecoverable error occurred.
};

/// Convert a RuntimeState to a human-readable string.
inline const char* runtime_state_to_string(RuntimeState state) {
    switch (state) {
        case RuntimeState::Created:      return "Created";
        case RuntimeState::Initializing: return "Initializing";
        case RuntimeState::Ready:        return "Ready";
        case RuntimeState::Running:      return "Running";
        case RuntimeState::Paused:       return "Paused";
        case RuntimeState::Stopping:     return "Stopping";
        case RuntimeState::Stopped:      return "Stopped";
        case RuntimeState::Error:        return "Error";
    }
    return "Unknown";
}

} // namespace liz