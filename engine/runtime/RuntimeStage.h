#pragma once

#include <string>

namespace liz {

/// Fine-grained stages within the initialization and shutdown sequences.
///
/// The runtime progresses through these stages during initialize() and
/// shutdown().  Each stage corresponds to a distinct subsystem setup step.
enum class RuntimeStage {
    Boot,                  // Initial setup, logger started.
    LoadConfiguration,     // Config defaults loaded.
    InitializeResources,   // ResourceManager created.
    InitializeGPU,         // GPU context prepared.
    InitializeAI,          // AI subsystems prepared.
    InitializePlugins,     // Plugin system activated.
    StartPipeline,         // Video/AI pipelines ready.
    Idle,                  // Runtime is idle, ready to run.
    Shutdown               // Shutdown sequence in progress.
};

/// Convert a RuntimeStage to a human-readable string.
inline const char* runtime_stage_to_string(RuntimeStage stage) {
    switch (stage) {
        case RuntimeStage::Boot:                 return "Boot";
        case RuntimeStage::LoadConfiguration:    return "LoadConfiguration";
        case RuntimeStage::InitializeResources:  return "InitializeResources";
        case RuntimeStage::InitializeGPU:        return "InitializeGPU";
        case RuntimeStage::InitializeAI:         return "InitializeAI";
        case RuntimeStage::InitializePlugins:    return "InitializePlugins";
        case RuntimeStage::StartPipeline:        return "StartPipeline";
        case RuntimeStage::Idle:                 return "Idle";
        case RuntimeStage::Shutdown:             return "Shutdown";
    }
    return "Unknown";
}

} // namespace liz