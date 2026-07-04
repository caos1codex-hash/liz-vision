#pragma once

#include "engine/ai/inference/Model.h"
#include "engine/ai/inference/ModelLoader.h"
#include "engine/gpu/GPUContext.h"

#include <chrono>
#include <cstddef>
#include <memory>
#include <string>
#include <vector>

namespace liz {

// Forward declaration — avoids pulling Engine.h into the inference header.
class Engine;

/// Statistics from an inference engine run.
struct InferenceStats {
    std::size_t frames_processed   = 0;
    std::size_t models_used        = 0;
    std::size_t inference_calls    = 0;
    std::size_t inference_failures = 0;
    double      total_time_ms      = 0.0;
    double      avg_inference_ms   = 0.0;
    std::string backend_name       = "CPU";
};

/// The central inference orchestrator for LIZ Vision.
///
/// Responsibilities:
///   - Manage loaded models (via ModelLoader)
///   - Route frames through one or more models sequentially
///   - Integrate with GPUContext for device routing
///   - Create Tasks for the Scheduler (via Engine)
///   - Measure inference timing
///
/// Flow per frame:
///   Frame -> Model.run() -> GPUContext (upload/execute/download)
///          -> Task System -> Scheduler -> Output Frame
///
/// In this sprint: everything is simulated. No real ML inference.
class InferenceEngine {
public:
    InferenceEngine() = default;
    ~InferenceEngine();

    // Non-copyable.
    InferenceEngine(const InferenceEngine&) = delete;
    InferenceEngine& operator=(const InferenceEngine&) = delete;

    // -- Lifecycle ---------------------------------------------------------------
    /// Initialize the engine: discover models, set up GPU context.
    /// @param gpu_ctx  Reference to an initialized GPUContext.
    /// @return true on success.
    bool initialize(GPUContext& gpu_ctx);

    /// Shut down: unload all models.
    void shutdown();

    /// True after a successful initialize().
    bool is_initialized() const;

    // -- Model management -------------------------------------------------------
    /// Load a model by name into the engine.
    /// Returns true if the model was found and loaded successfully.
    bool load_model(std::string_view name);

    /// Unload a model by name.
    /// Calls model->on_unload() and removes it.
    bool unload_model(std::string_view name);

    /// List the names of currently loaded models.
    std::vector<std::string> loaded_model_names() const;

    /// Number of models currently loaded.
    std::size_t model_count() const;

    // -- Inference --------------------------------------------------------------
    /// Process a single frame through ALL loaded models (chain).
    /// The output of model N becomes the input of model N+1.
    /// Returns the final enhanced frame.
    ///
    /// This is a direct (non-task) call — used for simple one-off inference.
    VideoFrame infer(const VideoFrame& input_frame);

    /// Process a vector of frames through all loaded models,
    /// submitting each (frame, model) pair as a Task to the Engine.
    /// Returns inference statistics.
    ///
    /// This is the integrated path: Frame -> Model -> GPU -> Task -> Scheduler.
    InferenceStats run_pipeline(Engine& engine,
                                 const std::vector<VideoFrame>& frames);

    // -- Accessors ---------------------------------------------------------------
    /// Reference to the underlying ModelLoader.
    ModelLoader&       loader();
    const ModelLoader& loader() const;

    /// Statistics from the last run_pipeline() call.
    const InferenceStats& last_stats() const;

    /// The GPU backend name being used.
    std::string gpu_backend_name() const;

private:
    GPUContext*                      gpu_ctx_   = nullptr;
    ModelLoader                      loader_;
    std::vector<std::unique_ptr<Model>> loaded_models_;
    bool                             initialized_ = false;
    InferenceStats                   stats_;
};

} // namespace liz