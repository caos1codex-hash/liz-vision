#include "engine/api/EngineSession.h"

#include <chrono>
#include <cstdint>
#include <random>
#include <sstream>

namespace liz {

// ── Constructor ───────────────────────────────────────────────────────

EngineSession::EngineSession()
    : created_at_(std::chrono::steady_clock::now()) {
    // Generate a random 8-hex UUID.
    auto seed = static_cast<std::uint64_t>(
        std::chrono::steady_clock::now().time_since_epoch().count());
    std::mt19937_64 rng(seed);
    std::uniform_int_distribution<std::uint64_t> dist(0, 0xFFFF'FFFF);

    static constexpr int kIdLength = 8;
    char buf[kIdLength + 1];
    for (int i = 0; i < kIdLength; ++i) {
        std::snprintf(buf + i, 2, "%x", static_cast<int>(dist(rng) & 0xF));
    }
    buf[kIdLength] = '\0';
    id_ = buf;
}

EngineSession::EngineSession(std::string session_id)
    : id_(std::move(session_id))
    , created_at_(std::chrono::steady_clock::now()) {}

// ── Identity ──────────────────────────────────────────────────────────

const std::string& EngineSession::id() const {
    return id_;
}

// ── State ─────────────────────────────────────────────────────────────

bool EngineSession::is_running() const {
    return running_;
}

void EngineSession::shutdown() {
    running_ = false;
}

// ── Timing ────────────────────────────────────────────────────────────

double EngineSession::active_time_ms() const {
    auto now = std::chrono::steady_clock::now();
    return static_cast<double>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            now - created_at_).count());
}

// ── Info ───────────────────────────────────────────────────────────────

std::string EngineSession::to_string() const {
    std::ostringstream oss;
    oss << "Session{id=" << id_
        << ", running=" << (running_ ? "true" : "false")
        << ", active_ms=" << active_time_ms() << "}";
    return oss.str();
}

} // namespace liz
