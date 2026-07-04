#pragma once

#include "engine/events/EventType.h"

#include <cstdint>
#include <string>

namespace liz {

/// An immutable event that flows through the EventBus.
///
/// Each event carries a unique UUID, a timestamp, a type, a source
/// identifier, a human-readable message, and a priority level.
class Event {
public:
    Event(EventType type,
          std::string source,
          std::string message,
          EventPriority priority = EventPriority::Normal);

    /// Unique identifier for this event instance.
    const std::string& uuid() const;

    /// Monotonic timestamp (milliseconds since some epoch).
    std::uint64_t timestamp() const;

    /// The type of this event.
    EventType type() const;

    /// A human-readable name for the event type.
    const char* type_string() const;

    /// The module or component that published this event.
    const std::string& source() const;

    /// A descriptive message attached to the event.
    const std::string& message() const;

    /// The priority of this event.
    EventPriority priority() const;

    /// A human-readable name for the priority level.
    const char* priority_string() const;

    /// Produce a summary string suitable for logging.
    std::string info() const;

private:
    std::string    uuid_;
    std::uint64_t  timestamp_;
    EventType      type_;
    std::string    source_;
    std::string    message_;
    EventPriority  priority_;

    /// Generate a simple unique ID (no external dependency).
    static std::string generate_uuid();
};

} // namespace liz