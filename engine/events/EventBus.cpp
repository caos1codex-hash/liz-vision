#include "engine/events/EventBus.h"
#include "engine/core/Logger.h"

#include <algorithm>
#include <sstream>

namespace liz {

EventBus::EventBus() = default;

bool EventBus::subscribe(EventListener* listener) {
    if (!listener) return false;

    // Reject duplicates (by pointer).
    for (const auto* existing : listeners_) {
        if (existing == listener) {
            return false;
        }
    }

    listeners_.push_back(listener);

    std::ostringstream oss;
    oss << "EventBus: listener '"
        << listener->name()
        << "' subscribed ("
        << listeners_.size() << " total)";
    LIZ_INFO(oss.str());

    return true;
}

bool EventBus::unsubscribe(EventListener* listener) {
    if (!listener) return false;

    auto it = std::find(listeners_.begin(), listeners_.end(), listener);
    if (it == listeners_.end()) {
        return false;
    }

    std::ostringstream oss;
    oss << "EventBus: listener '"
        << listener->name()
        << "' unsubscribed ("
        << (listeners_.size() - 1) << " remaining)";
    LIZ_INFO(oss.str());

    listeners_.erase(it);
    return true;
}

std::size_t EventBus::publish(const Event& event) {
    ++total_published_;

    std::ostringstream pub_oss;
    pub_oss << "EventBus: published " << event.info()
             << " (" << listeners_.size() << " listeners)";
    LIZ_INFO(pub_oss.str());

    std::size_t before = listeners_.size();
    std::size_t delivered = dispatcher_.dispatch(event, listeners_);

    // Count as failed any listener that didn't get the event
    // (shouldn't happen with sync dispatch, but for accounting).
    std::size_t failed = before - delivered;
    total_delivered_ += delivered;
    total_failed_    += failed;

    if (delivered > 0) {
        std::ostringstream del_oss;
        del_oss << "EventBus: delivered " << event.type_string()
                 << " to " << delivered << " listener(s)";
        LIZ_INFO(del_oss.str());
    }

    return delivered;
}

std::size_t EventBus::listener_count() const {
    return listeners_.size();
}

std::size_t EventBus::event_count() const {
    return total_published_;
}

void EventBus::clear() {
    std::ostringstream oss;
    oss << "EventBus: clearing " << listeners_.size() << " listener(s)";
    LIZ_INFO(oss.str());
    listeners_.clear();
}

EventBusStats EventBus::statistics() const {
    return {
        total_published_,
        total_delivered_,
        total_failed_,
        listeners_.size()
    };
}

void EventBus::log_statistics() const {
    auto s = statistics();
    std::ostringstream oss;
    oss << "EventBus stats: published=" << s.total_published
        << " delivered=" << s.total_delivered
        << " failed=" << s.total_failed
        << " listeners=" << s.listener_count;
    LIZ_INFO(oss.str());
}

} // namespace liz