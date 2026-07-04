#include "engine/runtime/LifecycleManager.h"
#include "engine/core/Logger.h"

#include <sstream>

namespace liz {

LifecycleManager::LifecycleManager() = default;

bool LifecycleManager::transition_to(RuntimeState target) {
    if (!is_valid_transition(current_state_, target)) {
        std::ostringstream oss;
        oss << "Invalid state transition: "
            << runtime_state_to_string(current_state_)
            << " -> "
            << runtime_state_to_string(target);
        LIZ_ERROR(oss.str());
        return false;
    }

    previous_state_ = current_state_;
    current_state_ = target;
    ++transition_count_;

    std::ostringstream oss;
    oss << "State transition: "
        << runtime_state_to_string(previous_state_)
        << " -> "
        << runtime_state_to_string(current_state_);
    LIZ_INFO(oss.str());

    return true;
}

RuntimeState LifecycleManager::state() const {
    return current_state_;
}

const char* LifecycleManager::state_string() const {
    return runtime_state_to_string(current_state_);
}

RuntimeState LifecycleManager::previous_state() const {
    return previous_state_;
}

std::size_t LifecycleManager::transition_count() const {
    return transition_count_;
}

bool LifecycleManager::is_valid_transition(RuntimeState from, RuntimeState to) {
    // Any state can go to Error.
    if (to == RuntimeState::Error) {
        return true;
    }

    switch (from) {
        case RuntimeState::Created:
            return to == RuntimeState::Initializing;

        case RuntimeState::Initializing:
            return to == RuntimeState::Ready;

        case RuntimeState::Ready:
            return to == RuntimeState::Running;

        case RuntimeState::Running:
            return to == RuntimeState::Paused || to == RuntimeState::Stopping;

        case RuntimeState::Paused:
            return to == RuntimeState::Running;

        case RuntimeState::Stopping:
            return to == RuntimeState::Stopped;

        case RuntimeState::Stopped:
            return false;  // Terminal state.

        case RuntimeState::Error:
            return false;  // Terminal state.
    }

    return false;
}

} // namespace liz