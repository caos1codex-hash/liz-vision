#include "engine/core/EngineContext.h"
#include "engine/core/Logger.h"
#include "engine/core/Config.h"

#include <chrono>
#include <cstdint>
#include <random>
#include <sstream>

namespace liz {

// ── Subsystem access ─────────────────────────────────────────────────────────
Logger& EngineContext::logger() {
    return Logger::instance();
}

const Logger& EngineContext::logger() const {
    return Logger::instance();
}

Config& EngineContext::config() {
    return Config::instance();
}

const Config& EngineContext::config() const {
    return Config::instance();
}

// ── State management ─────────────────────────────────────────────────────────
EngineState EngineContext::state() const {
    return state_;
}

void EngineContext::set_state(EngineState new_state) {
    auto old_state = state_;
    state_ = new_state;

    if (old_state != new_state) {
        std::ostringstream oss;
        oss << "Engine state: "
            << engine_state_to_string(old_state)
            << " -> "
            << engine_state_to_string(new_state);
        LIZ_INFO(oss.str());
    }
}

bool EngineContext::is_operational() const {
    return state_ == EngineState::Ready || state_ == EngineState::Running;
}

// ── Session metadata ─────────────────────────────────────────────────────────
const std::string& EngineContext::session_id() const {
    return session_id_;
}

void EngineContext::init_session() {
    // Generate a short random hex session ID (8 chars).
    static constexpr int kIdLength = 8;

    auto seed = static_cast<std::uint64_t>(
        std::chrono::steady_clock::now().time_since_epoch().count());
    std::mt19937_64 rng(seed);
    std::uniform_int_distribution<std::uint64_t> dist(0, 0xFFFF'FFFF);

    char buf[kIdLength + 1];
    for (int i = 0; i < kIdLength; ++i) {
        std::snprintf(buf + i, 2, "%x", static_cast<int>(dist(rng) & 0xF));
    }
    buf[kIdLength] = '\0';
    session_id_ = buf;

    set_state(EngineState::Uninitialized);
}

// ── Free function ────────────────────────────────────────────────────────────
const char* engine_state_to_string(EngineState state) {
    switch (state) {
        case EngineState::Uninitialized: return "Uninitialized";
        case EngineState::Initializing:  return "Initializing";
        case EngineState::Ready:        return "Ready";
        case EngineState::Running:      return "Running";
        case EngineState::Pausing:      return "Pausing";
        case EngineState::Paused:       return "Paused";
        case EngineState::ShuttingDown: return "ShuttingDown";
        case EngineState::Stopped:      return "Stopped";
    }
    return "Unknown";
}

} // namespace liz