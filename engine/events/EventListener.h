#pragma once

#include "engine/events/Event.h"

namespace liz {

/// Abstract interface for event listeners.
///
/// Implementations receive events via on_event() and can react
/// to specific event types as needed.
class EventListener {
public:
    virtual ~EventListener() = default;

    /// Called when an event is dispatched to this listener.
    /// Implementations should be lightweight and non-blocking.
    virtual void on_event(const Event& event) = 0;

    /// Optional: return a human-readable name for this listener.
    /// Default returns "UnnamedListener".
    virtual std::string_view name() const {
        return "UnnamedListener";
    }
};

} // namespace liz