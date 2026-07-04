#include "engine/runtime/Runtime.h"

#include <sstream>

namespace liz {

Runtime::Runtime() = default;

Runtime::~Runtime() {
    // Best-effort shutdown if not already stopped.
    if (lifecycle_.state() != RuntimeState::Stopped &&
        lifecycle_.state() != RuntimeState::Created) {
        shutdown();
    }
}

bool Runtime::initialize() {
    // Transition: Created -> Initializing
    if (!lifecycle_.transition_to(RuntimeState::Initializing)) {
        return false;
    }

    // ── Stage: Boot ───────────────────────────────────────────────────────────
    set_stage(RuntimeStage::Boot);
    {
        std::ostringstream oss;
        oss << "Runtime initializing (session will be assigned by Engine)";
        LIZ_INFO(oss.str());
    }

    // ── Stage: LoadConfiguration ──────────────────────────────────────────────
    set_stage(RuntimeStage::LoadConfiguration);
    Config::instance().load_defaults();
    LIZ_INFO("Configuration loaded (defaults)");

    // ── Stage: InitializeResources ────────────────────────────────────────────
    set_stage(RuntimeStage::InitializeResources);
    {
        std::ostringstream oss;
        oss << "Resource Manager ready";
        LIZ_INFO(oss.str());
    }

    // ── Stage: InitializeGPU ──────────────────────────────────────────────────
    set_stage(RuntimeStage::InitializeGPU);
    LIZ_INFO("GPU subsystem placeholder initialized");

    // ── Stage: InitializeAI ───────────────────────────────────────────────────
    set_stage(RuntimeStage::InitializeAI);
    LIZ_INFO("AI subsystem placeholder initialized");

    // ── Stage: InitializePlugins ──────────────────────────────────────────────
    set_stage(RuntimeStage::InitializePlugins);
    LIZ_INFO("Plugin subsystem placeholder initialized");

    // ── Stage: StartPipeline ──────────────────────────────────────────────────
    set_stage(RuntimeStage::StartPipeline);
    LIZ_INFO("Pipeline subsystem placeholder initialized");

    // ── Stage: Idle ───────────────────────────────────────────────────────────
    set_stage(RuntimeStage::Idle);

    // Transition: Initializing -> Ready
    if (!lifecycle_.transition_to(RuntimeState::Ready)) {
        lifecycle_.transition_to(RuntimeState::Error);
        return false;
    }

    LIZ_INFO("Runtime is Ready");
    return true;
}

bool Runtime::run() {
    if (!lifecycle_.transition_to(RuntimeState::Running)) {
        return false;
    }
    LIZ_INFO("Runtime is now Running");
    return true;
}

bool Runtime::pause() {
    if (!lifecycle_.transition_to(RuntimeState::Paused)) {
        return false;
    }
    LIZ_INFO("Runtime is now Paused");
    return true;
}

bool Runtime::resume() {
    if (!lifecycle_.transition_to(RuntimeState::Running)) {
        return false;
    }
    LIZ_INFO("Runtime resumed, now Running");
    return true;
}

bool Runtime::shutdown() {
    // Allow shutdown from Running or Paused.
    if (lifecycle_.state() == RuntimeState::Running ||
        lifecycle_.state() == RuntimeState::Paused) {

        if (!lifecycle_.transition_to(RuntimeState::Stopping)) {
            return false;
        }

        set_stage(RuntimeStage::Shutdown);
        LIZ_INFO("Runtime shutting down subsystems...");

        engine_.shutdown();
        LIZ_INFO("Engine shut down");

        resource_manager_.clear_cache();
        LIZ_INFO("Resource cache cleared");

        lifecycle_.transition_to(RuntimeState::Stopped);
        LIZ_INFO("Runtime is Stopped");
        return true;
    }

    // Already stopped or in an invalid state for shutdown.
    {
        std::ostringstream oss;
        oss << "Cannot shutdown from state: "
            << runtime_state_to_string(lifecycle_.state());
        LIZ_ERROR(oss.str());
    }
    return false;
}

RuntimeState Runtime::state() const {
    return lifecycle_.state();
}

RuntimeStage Runtime::stage() const {
    return current_stage_;
}

const char* Runtime::state_string() const {
    return lifecycle_.state_string();
}

const char* Runtime::stage_string() const {
    return runtime_stage_to_string(current_stage_);
}

void Runtime::status() const {
    std::ostringstream oss;
    oss << "Runtime status: state="
        << runtime_state_to_string(lifecycle_.state())
        << ", stage="
        << runtime_stage_to_string(current_stage_)
        << ", transitions="
        << lifecycle_.transition_count()
        << ", resources="
        << resource_manager_.resource_count();
    LIZ_INFO(oss.str());
}

Logger& Runtime::logger() {
    return Logger::instance();
}

Config& Runtime::config() {
    return Config::instance();
}

ResourceManager& Runtime::resource_manager() {
    return resource_manager_;
}

Engine& Runtime::engine() {
    return engine_;
}

void Runtime::set_stage(RuntimeStage stage) {
    current_stage_ = stage;
    std::ostringstream oss;
    oss << "Stage: " << runtime_stage_to_string(stage);
    LIZ_INFO(oss.str());
}

} // namespace liz