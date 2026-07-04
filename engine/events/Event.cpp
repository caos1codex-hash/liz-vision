#include "engine/events/Event.h"

#include <atomic>
#include <chrono>
#include <sstream>

namespace liz {

Event::Event(EventType type,
             std::string source,
             std::string message,
             EventPriority priority)
    : uuid_(generate_uuid())
    , timestamp_(static_cast<std::uint64_t>(
          std::chrono::steady_clock::now()
              .time_since_epoch()
              .count()))
    , type_(type)
    , source_(std::move(source))
    , message_(std::move(message))
    , priority_(priority) {}

const std::string& Event::uuid() const { return uuid_; }

std::uint64_t Event::timestamp() const { return timestamp_; }

EventType Event::type() const { return type_; }

const char* Event::type_string() const {
    return event_type_to_string(type_);
}

const std::string& Event::source() const { return source_; }

const std::string& Event::message() const { return message_; }

EventPriority Event::priority() const { return priority_; }

const char* Event::priority_string() const {
    return event_priority_to_string(priority_);
}

std::string Event::info() const {
    std::ostringstream oss;
    oss << "Event[" << type_string()
        << "] src=" << source_
        << " msg=" << message_
        << " prio=" << priority_string();
    return oss.str();
}

std::string Event::generate_uuid() {
    static std::atomic<std::uint64_t> counter{0};
    ++counter;
    std::ostringstream oss;
    oss << "evt-" << counter.load();
    return oss.str();
}

} // namespace liz