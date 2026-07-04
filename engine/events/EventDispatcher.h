#pragma once

#include "engine/events/Event.h"
#include "engine/events/EventListener.h"

#include <cstddef>
#include <vector>

namespace liz {

/// Delivers a single event to all registered listeners in registration order.
///
/// This is a synchronous, single-threaded dispatcher.
/// It does NOT filter by event type — all listeners receive all events.
/// Filtering is the responsibility of each listener's on_event() implementation.
class EventDispatcher {
public:
    EventDispatcher() = default;

    /// Dispatch the event to every registered listener, in order.
    /// Returns the number of listeners that received the event.
    std::size_t dispatch(const Event& event,
                         const std::vector<EventListener*>& listeners) const;

private:
    EventDispatcher(const EventDispatcher&) = delete;
    EventDispatcher& operator=(const EventDispatcher&) = delete;
};

} // namespace liz