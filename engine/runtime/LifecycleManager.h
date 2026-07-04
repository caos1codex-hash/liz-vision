#pragma once

#include "engine/runtime/RuntimeState.h"

#include <string>

namespace liz {

/// Validates and executes state transitions for the Runtime.
///
/// This class is the single authority on whether a transition from one
/// RuntimeState to another is legal.  Invalid transitions are logged as
/// errors and rejected.
///
/// Valid transitions:
///   Created      -> Initializing
///   Initializing -> Ready
///   Ready        -> Running
///   Running      -> Paused
///   Paused       -> Running
///   Running      -> Stopping
///   Stopping     -> Stopped
///
/// Any state can transition to Error on a fatal failure.
class LifecycleManager {
public:
    LifecycleManager();

    /// Attempt to transition to the target state.
    /// Returns true if the transition was valid and applied.
    /// Returns false and logs an error if the transition is invalid.
    bool transition_to(RuntimeState target);

    /// Get the current runtime state.
    RuntimeState state() const;

    /// Get the current state as a human-readable string.
    const char* state_string() const;

    /// Get the previous state (before the last successful transition).
    RuntimeState previous_state() const;

    /// Get the number of successful transitions so far.
    std::size_t transition_count() const;

private:
    /// Check whether transitioning from `from` to `to` is allowed.
    static bool is_valid_transition(RuntimeState from, RuntimeState to);

    RuntimeState  current_state_ = RuntimeState::Created;
    RuntimeState  previous_state_ = RuntimeState::Created;
    std::size_t   transition_count_ = 0;
};

} // namespace liz