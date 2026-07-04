#pragma once

#include "engine/core/EngineContext.h"
#include "engine/plugins/PluginManager.h"
#include "engine/scheduler/Scheduler.h"

#include <memory>
#include <string>

namespace liz {

/// The central orchestrator of LIZ Vision.
///
/// Owns the EngineContext, Scheduler, and PluginManager.
/// Provides the single entry point for initialization, task submission,
/// and shutdown.
///
/// Usage:
///   Engine engine;
///   engine.initialize();
///   engine.submit_task("my_task", []{ ... });
///   engine.run_pending();
///   engine.shutdown();
class Engine {
public:
    Engine();
    ~Engine();

    // Non-copyable, non-movable — single owner of all subsystems.
    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;

    // ── Lifecycle ─────────────────────────────────────────────────────────────
    /// Initialize all subsystems (Config, Logger, Context, Scheduler, Plugins).
    /// Must be called before any other operation.  Returns true on success.
    bool initialize();

    /// Gracefully shut down the engine.
    /// Unregisters all plugins, drains no more tasks, sets state to Stopped.
    void shutdown();

    // ── Task submission ───────────────────────────────────────────────────────
    /// Submit a named task to the scheduler.  Returns the assigned TaskId.
    TaskId submit_task(std::string name, Task::ExecuteFn fn);

    /// Execute the next pending task.  Returns true if a task was run.
    bool run_next();

    /// Execute all pending tasks.  Returns the count executed.
    std::size_t run_pending();

    // ── Plugin management ────────────────────────────────────────────────────
    /// Register a plugin with the engine.
    bool register_plugin(std::shared_ptr<PluginInterface> plugin);

    /// Unregister a plugin by name.
    bool unregister_plugin(std::string_view name);

    // ── Accessors ────────────────────────────────────────────────────────────
    EngineContext&       context();
    const EngineContext& context() const;

    Scheduler&       scheduler();
    const Scheduler& scheduler() const;

    PluginManager&       plugin_manager();
    const PluginManager& plugin_manager() const;

    /// The engine session identifier (available after initialize()).
    const std::string& session_id() const;

private:
    EngineContext  context_;
    Scheduler      scheduler_;
    PluginManager  plugin_manager_;
    bool           initialized_ = false;
};

} // namespace liz