#include "engine/core/Engine.h"
#include "engine/core/Config.h"
#include "engine/core/Logger.h"

#include <sstream>

namespace liz {

// ── Constructor / Destructor ─────────────────────────────────────────────────
Engine::Engine() = default;

Engine::~Engine() {
    if (initialized_) {
        shutdown();
    }
}

// ── Lifecycle ─────────────────────────────────────────────────────────────────
bool Engine::initialize() {
    if (initialized_) {
        LIZ_WARN("Engine: already initialized");
        return true;
    }

    LIZ_INFO("=== LIZ Vision Engine initializing ===");

    // 1. Initialize session ID and state
    context_.init_session();
    context_.set_state(EngineState::Initializing);

    // 2. Load default configuration
    context_.config().load_defaults();

    std::ostringstream oss;
    oss << "Engine session: " << context_.session_id();
    LIZ_INFO(oss.str());

    // 3. Apply log level from config
    auto log_level_str = context_.config().get("engine.log_level", "INFO");
    if (log_level_str == "TRACE") {
        Logger::instance().set_level(LogLevel::Trace);
    } else if (log_level_str == "DEBUG") {
        Logger::instance().set_level(LogLevel::Debug);
    } else if (log_level_str == "WARN") {
        Logger::instance().set_level(LogLevel::Warn);
    } else if (log_level_str == "ERROR") {
        Logger::instance().set_level(LogLevel::Error);
    } else {
        Logger::instance().set_level(LogLevel::Info);
    }

    LIZ_INFO("Config loaded: engine=" + context_.config().get("engine.name", "") +
             " v" + context_.config().get("engine.version", ""));

    // 4. Mark as ready
    context_.set_state(EngineState::Ready);
    initialized_ = true;

    LIZ_INFO("=== LIZ Vision Engine ready ===");
    return true;
}

void Engine::shutdown() {
    if (!initialized_) {
        LIZ_WARN("Engine: shutdown called but engine was not initialized");
        return;
    }

    LIZ_INFO("=== LIZ Vision Engine shutting down ===");
    context_.set_state(EngineState::ShuttingDown);

    // Unregister all plugins
    plugin_manager_.unregister_all();

    // Drain remaining tasks (best-effort)
    auto remaining = scheduler_.pending_count();
    if (remaining > 0) {
        std::ostringstream oss;
        oss << "Engine: draining " << remaining << " remaining tasks before shutdown";
        LIZ_WARN(oss.str());
        scheduler_.run_all();
    }

    context_.set_state(EngineState::Stopped);
    initialized_ = false;

    LIZ_INFO("=== LIZ Vision Engine stopped ===");
}

// ── Task submission ───────────────────────────────────────────────────────────
TaskId Engine::submit_task(std::string name, Task::ExecuteFn fn) {
    if (!initialized_) {
        LIZ_ERROR("Engine: cannot submit task — engine not initialized");
        return 0;
    }

    return scheduler_.submit(std::move(name), std::move(fn));
}

bool Engine::run_next() {
    if (!initialized_) {
        LIZ_ERROR("Engine: cannot run task — engine not initialized");
        return false;
    }

    return scheduler_.run_next();
}

std::size_t Engine::run_pending() {
    if (!initialized_) {
        LIZ_ERROR("Engine: cannot run tasks — engine not initialized");
        return 0;
    }

    return scheduler_.run_all();
}

// ── Plugin management ────────────────────────────────────────────────────────
bool Engine::register_plugin(std::shared_ptr<PluginInterface> plugin) {
    if (!initialized_) {
        LIZ_ERROR("Engine: cannot register plugin — engine not initialized");
        return false;
    }

    return plugin_manager_.register_plugin(std::move(plugin));
}

bool Engine::unregister_plugin(std::string_view name) {
    return plugin_manager_.unregister(name);
}

// ── Accessors ────────────────────────────────────────────────────────────────
EngineContext& Engine::context() { return context_; }
const EngineContext& Engine::context() const { return context_; }

Scheduler& Engine::scheduler() { return scheduler_; }
const Scheduler& Engine::scheduler() const { return scheduler_; }

PluginManager& Engine::plugin_manager() { return plugin_manager_; }
const PluginManager& Engine::plugin_manager() const { return plugin_manager_; }

const std::string& Engine::session_id() const { return context_.session_id(); }

} // namespace liz