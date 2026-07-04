#pragma once

#include <chrono>
#include <string>

namespace liz {

/// Represents a public session of the LIZ Engine.
///
/// Each session has a unique identifier, a creation timestamp,
/// a state, and tracks how long it has been active.
///
/// Sessions are created and destroyed through the EngineAPI facade.
/// External applications should NEVER access the internal Engine directly.
class EngineSession {
public:
    EngineSession();

    /// Construct with a specific session ID.
    explicit EngineSession(std::string session_id);

    ~EngineSession() = default;

    // Non-copyable.
    EngineSession(const EngineSession&) = delete;
    EngineSession& operator=(const EngineSession&) = delete;

    // Movable.
    EngineSession(EngineSession&&) noexcept = default;
    EngineSession& operator=(EngineSession&&) noexcept = default;

    // ── Identity ──────────────────────────────────────────────────────

    /// Unique session identifier (UUID).
    const std::string& id() const;

    // ── State ──────────────────────────────────────────────────────────

    /// True if the session is active (not shut down).
    bool is_running() const;

    /// Shut down the session.  Marks it as no longer active.
    void shutdown();

    // ── Timing ─────────────────────────────────────────────────────────

    /// How long this session has been active, in milliseconds.
    double active_time_ms() const;

    // ── Info ───────────────────────────────────────────────────────────

    /// Generate a SessionInfo-like summary string.
    std::string to_string() const;

private:
    std::string   id_;
    bool          running_ = true;
    std::chrono::steady_clock::time_point created_at_;
};

} // namespace liz
