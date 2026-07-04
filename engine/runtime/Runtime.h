#pragma once

#include "engine/runtime/RuntimeState.h"
#include "engine/runtime/RuntimeStage.h"
#include "engine/runtime/LifecycleManager.h"

#include "engine/core/Logger.h"
#include "engine/core/Config.h"
#include "engine/core/Engine.h"
#include "engine/resources/ResourceManager.h"

#include <memory>
#include <string>

namespace liz {

/// The central runtime of LIZ Vision.
///
/// Runtime is the single entry point for the engine lifecycle.
/// It owns and coordinates all subsystems: Logger, Config, ResourceManager,
/// and Engine.  No module should control its own lifecycle — everything
/// goes through Runtime.
///
/// This sprint (Sprint 10) implements the lifecycle control ONLY.
/// No actual video processing, AI inference, or GPU work is performed.
class Runtime {
public:
    Runtime();
    ~Runtime();

    // Non-copyable, non-movable.
    Runtime(const Runtime&) = delete;
    Runtime& operator=(const Runtime&) = delete;

    // ── Lifecycle ─────────────────────────────────────────────────────────────

    /// Initialize all subsystems in the defined stage order.
    /// Transitions: Created -> Initializing -> Ready.
    /// Returns true on success, false on error (state becomes Error).
    bool initialize();

    /// Transition from Ready to Running.
    /// Returns true if the runtime is now in Running state.
    bool run();

    /// Transition from Running to Paused.
    /// Returns true if the runtime is now in Paused state.
    bool pause();

    /// Transition from Paused to Running.
    /// Returns true if the runtime is now in Running state.
    bool resume();

    /// Transition from Running/Paused to Stopping -> Stopped.
    /// Returns true on success.
    bool shutdown();

    // ── Status ────────────────────────────────────────────────────────────────

    /// Get the current RuntimeState.
    RuntimeState state() const;

    /// Get the current RuntimeStage.
    RuntimeStage stage() const;

    /// Get the current state as a readable string.
    const char* state_string() const;

    /// Get the current stage as a readable string.
    const char* stage_string() const;

    /// Print a summary of the current runtime status.
    void status() const;

    // ── Subsystem accessors ───────────────────────────────────────────────────

    Logger&          logger();
    Config&          config();
    ResourceManager& resource_manager();
    Engine&          engine();

private:
    /// Advance the runtime stage and log it.
    void set_stage(RuntimeStage stage);

    LifecycleManager   lifecycle_;
    RuntimeStage       current_stage_ = RuntimeStage::Boot;

    // Subsystems owned by Runtime.
    ResourceManager    resource_manager_;
    Engine             engine_;
};

} // namespace liz