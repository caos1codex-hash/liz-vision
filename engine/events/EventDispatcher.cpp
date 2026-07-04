#include "engine/events/EventDispatcher.h"
#include "engine/core/Logger.h"

#include <sstream>

namespace liz {

std::size_t EventDispatcher::dispatch(
    const Event& event,
    const std::vector<EventListener*>& listeners) const {

    std::size_t delivered = 0;

    for (auto* listener : listeners) {
        if (!listener) continue;
        try {
            listener->on_event(event);
            ++delivered;
        } catch (const std::exception& e) {
            std::ostringstream oss;
            oss << "EventDispatcher: listener '"
                << listener->name()
                << "' threw exception handling event "
                << event.type_string()
                << ": " << e.what();
            LIZ_ERROR(oss.str());
        } catch (...) {
            std::ostringstream oss;
            oss << "EventDispatcher: listener '"
                << listener->name()
                << "' threw unknown exception handling event "
                << event.type_string();
            LIZ_ERROR(oss.str());
        }
    }

    return delivered;
}

} // namespace liz