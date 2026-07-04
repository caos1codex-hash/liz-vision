#pragma once

#include "engine/events/Event.h"
#include "engine/events/EventListener.h"
#include "engine/events/EventDispatcher.h"

#include <cstddef>
#include <string>
#include <vector>

namespace liz {

/// EventBus statistics snapshot.
struct EventBusStats {
    std::size_t total_published   = 0;
    std::size_t total_delivered   = 0;
    std::size_t total_failed      = 0;
    std::size_t listener_count    = 0;
};

/// Central event bus for the LIZ Vision engine.
///
/// Modules publish events; listeners subscribe to receive them.
/// Delivery is synchronous and respects registration order.
///
/// Usage:
///   EventBus bus;
///   bus.subscribe(&my_listener);
///   bus.publish(Event(EventType::EngineStarted, "Runtime", "Engine is up"));
class EventBus {
public:
    EventBus();

    /// Subscribe a listener.  Returns true if added (no duplicates by pointer).
    bool subscribe(EventListener* listener);

    /// Unsubscribe a listener.  Returns true if found and removed.
    bool unsubscribe(EventListener* listener);

    /// Publish an event to all subscribed listeners.
    /// Returns the number of listeners that received the event.
    std::size_t publish(const Event& event);

    /// Number of currently subscribed listeners.
    std::size_t listener_count() const;

    /// Total number of events published so far.
    std::size_t event_count() const;

    /// Remove all listeners.
    void clear();

    /// Get a statistics snapshot.
    EventBusStats statistics() const;

    /// Log a summary of the event bus statistics.
    void log_statistics() const;

private:
    std::vector<EventListener*> listeners_;
    EventDispatcher             dispatcher_;
    std::size_t                 total_published_  = 0;
    std::size_t                 total_delivered_  = 0;
    std::size_t                 total_failed_     = 0;
};

} // namespace liz