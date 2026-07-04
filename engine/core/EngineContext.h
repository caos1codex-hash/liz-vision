#pragma once

#include <string>

namespace liz {

// Forward declarations — avoid heavy includes in the header.
class Logger;
class Config;

/// Represents the lifecycle state of the Engine.
enum class EngineState {
    Uninitialized,   ///< Created but init() not called yet
    Initializing,    ///< init() is in progress
    Ready,           ///< Fully initialized and ready to process
    Running,         ///< Actively processing tasks
    Pausing,         ///< Transitioning to paused
    Paused,          ///< Task execution is paused
    ShuttingDown,    ///< shutdown() is in progress
    Stopped          ///< Fully shut down
};

/// Converts an EngineState to a human-readable string.
const char* engine_state_to_string(EngineState state);

/// Central context object shared across all engine subsystems.
///
/// Holds references to the singletons (Logger, Config) and tracks
/// the global Engine state.  Every subsystem receives a const or
/// non-const reference to the EngineContext so it can access shared
/// resources without reaching for singletons directly.
class EngineContext {
public:
    EngineContext() = default;

    // ── Subsystem access ─────────────────────────────────────────────────────
    Logger&       logger();
    const Logger& logger() const;

    Config&       config();
    const Config& config() const;

    // ── State management ─────────────────────────────────────────────────────
    EngineState state() const;
    void         set_state(EngineState new_state);

    /// Convenience: returns true when the engine is in a state that allows
    /// task submission (Ready or Running).
    bool is_operational() const;

    // ── Engine metadata ──────────────────────────────────────────────────────
    /// Unique identifier for this engine session.
    const std::string& session_id() const;

private:
    friend class Engine;  // Only Engine may call init_session()

    /// Generate a session ID and set the initial state.
    /// Called once by Engine::initialize().
    void init_session();

    EngineState state_     = EngineState::Uninitialized;
    std::string session_id_;
};

} // namespace liz