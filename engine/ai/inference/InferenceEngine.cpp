#include "engine/ai/inference/InferenceEngine.h"
#include "engine/core/Engine.h"
#include "engine/core/Logger.h"

#include <algorithm>
#include <chrono>
#include <sstream>

namespace liz {

// -- Destructor -----------------------------------------------------------------
InferenceEngine::~InferenceEngine() {
    if (initialized_) {
        shutdown();
    }
}

// -- Lifecycle ------------------------------------------------------------------
bool InferenceEngine::initialize(GPUContext& gpu_ctx) {
    if (initialized_) {
        LIZ_WARN("InferenceEngine: already initialized");
        return true;
    }

    LIZ_INFO("InferenceEngine: initializing...");

    gpu_ctx_ = &gpu_ctx;

    // Discover available models.
    loader_.discover();

    initialized_ = true;

    std::ostringstream oss;
    oss << "InferenceEngine: ready — "
        << loader_.model_count() << " model(s) available, "
        << "GPU backend='" << gpu_ctx.backend_name() << "'";
    LIZ_INFO(oss.str());

    return true;
}

void InferenceEngine::shutdown() {
    if (!initialized_) return;

    LIZ_INFO("InferenceEngine: shutting down...");

    // Unload all models in reverse order.
    while (!loaded_models_.empty()) {
        auto& model = loaded_models_.back();
        model->on_unload();
        loaded_models_.pop_back();
    }

    gpu_ctx_      = nullptr;
    initialized_  = false;

    LIZ_INFO("InferenceEngine: shut down");
}

bool InferenceEngine::is_initialized() const {
    return initialized_;
}

// -- Model management -----------------------------------------------------------
bool InferenceEngine::load_model(std::string_view name) {
    if (!initialized_) {
        LIZ_ERROR("InferenceEngine: not initialized — cannot load model");
        return false;
    }

    // Check for duplicate.
    for (const auto& m : loaded_models_) {
        if (m->name() == name) {
            std::ostringstream oss;
            oss << "InferenceEngine: model '" << name << "' already loaded";
            LIZ_WARN(oss.str());
            return true;  // already loaded — not an error
        }
    }

    auto model = loader_.load(name);
    if (!model) {
        return false;
    }

    loaded_models_.push_back(std::move(model));

    std::ostringstream oss;
    oss << "InferenceEngine: loaded model '" << name
        << "' — " << loaded_models_.size() << " model(s) active";
    LIZ_INFO(oss.str());

    return true;
}

bool InferenceEngine::unload_model(std::string_view name) {
    auto it = std::find_if(loaded_models_.begin(), loaded_models_.end(),
                           [&](const auto& m) { return m->name() == name; });

    if (it == loaded_models_.end()) {
        std::ostringstream oss;
        oss << "InferenceEngine: model '" << name << "' not loaded";
        LIZ_WARN(oss.str());
        return false;
    }

    (*it)->on_unload();
    loaded_models_.erase(it);

    std::ostringstream oss;
    oss << "InferenceEngine: unloaded model '" << name
        << "' — " << loaded_models_.size() << " model(s) remaining";
    LIZ_INFO(oss.str());

    return true;
}

std::vector<std::string> InferenceEngine::loaded_model_names() const {
    std::vector<std::string> names;
    names.reserve(loaded_models_.size());
    for (const auto& m : loaded_models_) {
        names.emplace_back(m->name());
    }
    return names;
}

std::size_t InferenceEngine::model_count() const {
    return loaded_models_.size();
}

// -- Inference (direct) --------------------------------------------------------
VideoFrame InferenceEngine::infer(const VideoFrame& input_frame) {
    if (loaded_models_.empty()) {
        LIZ_WARN("InferenceEngine::infer: no models loaded — passthrough");
        return input_frame;
    }

    VideoFrame current = input_frame;

    for (const auto& model : loaded_models_) {
        auto start = std::chrono::steady_clock::now();
        current = model->run(current);
        auto end = std::chrono::steady_clock::now();

        double elapsed =
            std::chrono::duration<double, std::milli>(end - start).count();

        // Upload result to GPU (simulated routing).
        if (gpu_ctx_ && gpu_ctx_->is_initialized()) {
            auto handle = gpu_ctx_->upload_frame(current);
            if (handle) {
                gpu_ctx_->release_memory(handle);
            }
        }
    }

    return current;
}

// -- Inference (pipeline via Task System) ---------------------------------------
InferenceStats InferenceEngine::run_pipeline(Engine& engine,
                                               const std::vector<VideoFrame>& frames) {
    // Reset stats.
    stats_ = InferenceStats{};
    stats_.models_used = loaded_models_.size();
    stats_.backend_name = gpu_ctx_ ? gpu_ctx_->backend_name() : "none";

    LIZ_INFO("========== InferenceEngine PIPELINE START ==========");

    if (loaded_models_.empty()) {
        LIZ_WARN("InferenceEngine: no models loaded — frames pass through unchanged");
        stats_.frames_processed = frames.size();
        LIZ_INFO("========== InferenceEngine PIPELINE END (passthrough) ==========");
        return stats_;
    }

    if (frames.empty()) {
        LIZ_WARN("InferenceEngine: no frames to process");
        LIZ_INFO("========== InferenceEngine PIPELINE END (empty) ==========");
        return stats_;
    }

    auto total_start = std::chrono::steady_clock::now();

    // Submit tasks: one task per (frame, model) pair.
    // Each task simulates: model->run() + GPU routing.
    for (const auto& frame : frames) {
        for (const auto& model : loaded_models_) {
            // Capture by value for the lambda.
            auto fid    = frame.frame_id();
            auto fw     = frame.width();
            auto fh     = frame.height();
            auto fts    = frame.timestamp_ms();
            auto mname  = std::string(model->name());
            auto mtype  = std::string(model_type_to_string(model->type()));
            auto mver   = std::string(model->version());

            engine.submit_task(
                "Inference " + mname + " on Frame #" + std::to_string(fid),
                [fid, fw, fh, fts, mname, mtype, mver]() {
                    // Simulate inference timing.
                    auto start = std::chrono::steady_clock::now();

                    std::ostringstream oss;
                    oss << "running model " << mname
                        << " on Frame #" << fid
                        << " [" << fw << "x" << fh << "]"
                        << " ts=" << fts << "ms"
                        << " type=" << mtype
                        << " v" << mver
                        << " (simulated inference)";
                    LIZ_INFO(oss.str());

                    auto end = std::chrono::steady_clock::now();
                    double elapsed =
                        std::chrono::duration<double, std::milli>(end - start).count();

                    std::ostringstream oss2;
                    oss2 << "inference completed: " << mname
                         << " Frame #" << fid
                         << " in " << elapsed << " ms";
                    LIZ_DEBUG(oss2.str());

                    return true;
                }
            );
            ++stats_.inference_calls;
        }
    }

    {
        std::ostringstream oss;
        oss << "InferenceEngine: submitted " << stats_.inference_calls
            << " inference tasks ("
            << frames.size() << " frames x "
            << stats_.models_used << " models)";
        LIZ_INFO(oss.str());
    }

    // Execute all tasks via the Engine's Scheduler.
    auto executed = engine.run_pending();
    stats_.frames_processed   = frames.size();
    stats_.inference_failures = stats_.inference_calls - executed;

    auto total_end = std::chrono::steady_clock::now();
    stats_.total_time_ms =
        std::chrono::duration<double, std::milli>(total_end - total_start).count();

    if (stats_.inference_calls > 0) {
        stats_.avg_inference_ms =
            stats_.total_time_ms / static_cast<double>(stats_.inference_calls);
    }

    // Summary
    {
        std::ostringstream oss;
        oss << "InferenceEngine complete: "
            << stats_.frames_processed << " frames | "
            << stats_.models_used << " models | "
            << stats_.inference_calls << " calls | "
            << stats_.inference_failures << " failures | "
            << "backend='" << stats_.backend_name << "' | "
            << stats_.total_time_ms << " ms total | "
            << stats_.avg_inference_ms << " ms avg";
        LIZ_INFO(oss.str());
    }

    LIZ_INFO("========== InferenceEngine PIPELINE END ==========");
    return stats_;
}

// -- Accessors ------------------------------------------------------------------
ModelLoader&       InferenceEngine::loader()       { return loader_; }
const ModelLoader& InferenceEngine::loader() const { return loader_; }

const InferenceStats& InferenceEngine::last_stats() const {
    return stats_;
}

std::string InferenceEngine::gpu_backend_name() const {
    return gpu_ctx_ ? gpu_ctx_->backend_name() : "none";
}

} // namespace liz