// LIZ Vision — Desktop Application Entry Point
// Sprint 1+2+3+4+5 Demo: Engine, video pipeline, AI pipeline,
//   GPU routing, streaming, inference layer.

#include "engine/core/Engine.h"
#include "engine/core/Logger.h"
#include "engine/plugins/PluginInterface.h"
#include "engine/video/VideoPipeline.h"
#include "engine/video/VideoDecoder.h"
#include "engine/ai/AIPipeline.h"
#include "engine/ai/DemoProcessors.h"
#include "engine/ai/inference/InferenceEngine.h"
#include "engine/ai/inference/ModelLoader.h"
#include "engine/ai/models/ModelRegistry.h"
#include "engine/gpu/GPUContext.h"

#include <iostream>
#include <memory>
#include <sstream>

// -- Dummy Plugin (Sprint 1) ---------------------------------------------------
class DummyPlugin : public liz::PluginInterface {
public:
    explicit DummyPlugin(std::string name, std::string category = "test")
        : name_(std::move(name)), category_(std::move(category)) {}

    std::string_view name() const override { return name_; }
    std::string_view version() const override { return "0.1.0"; }
    std::string_view category() const override { return category_; }

    void on_register() override {
        std::ostringstream oss;
        oss << "DummyPlugin '" << name_ << "': on_register() called";
        LIZ_INFO(oss.str());
    }

    void on_unregister() override {
        std::ostringstream oss;
        oss << "DummyPlugin '" << name_ << "': on_unregister() called";
        LIZ_INFO(oss.str());
    }

private:
    std::string name_;
    std::string category_;
};

// -- Main ----------------------------------------------------------------------
int main() {
    std::cout << std::endl;

    // -- 1. Engine initialization -----------------------------------------------
    liz::Engine engine;
    if (!engine.initialize()) {
        std::cerr << "Failed to initialize LIZ Vision Engine" << std::endl;
        return 1;
    }

    std::cout << std::endl;

    // -- 2. Register plugins ---------------------------------------------------
    engine.register_plugin(std::make_shared<DummyPlugin>("Demo Upscaler", "upscaler"));
    engine.register_plugin(std::make_shared<DummyPlugin>("Demo Detector", "detection"));

    std::cout << std::endl;

    // -- 3. GPU Context --------------------------------------------------------
    LIZ_INFO("--- GPU Layer ---");

    liz::GPUContext gpu_ctx;
    if (!gpu_ctx.initialize()) {
        LIZ_ERROR("Failed to initialize GPU context");
        return 1;
    }
    gpu_ctx.log_routing_decision();

    std::cout << std::endl;

    // -- 4. Streaming Video Pipeline -------------------------------------------
    LIZ_INFO("--- Video Input Layer (streaming + GPU) ---");

    liz::VideoPipeline video_pipeline;

    auto vp_stats = video_pipeline.run_streaming(
        engine,
        "input_video.mp4",
        gpu_ctx
    );

    std::cout << std::endl;

    // -- 5. AI Pipeline (Sprint 3 — processor-based) ---------------------------
    LIZ_INFO("--- AI Processing Layer (Sprint 3 processors) ---");

    liz::AIPipeline ai_pipeline;
    ai_pipeline.context().load_defaults();
    ai_pipeline.add_processor(std::make_shared<liz::DemoUpscalerProcessor>());
    ai_pipeline.add_processor(std::make_shared<liz::DemoInterpolatorProcessor>());

    // Re-decode frames for AI pipeline demo.
    liz::VideoDecoder decoder;
    liz::DecodeRequest req;
    req.source_path   = "input_video.mp4";
    req.target_width  = 320;
    req.target_height = 240;
    req.fps           = 24.0;
    req.max_frames    = 5;

    auto decode_result = decoder.decode(req);
    liz::AIPipelineStats ai_stats{};
    if (decode_result.success) {
        ai_stats = ai_pipeline.run(engine, decode_result.frames);
    }

    std::cout << std::endl;

    // -- 6. Model Registry (Sprint 5 — catalog) --------------------------------
    LIZ_INFO("--- Model Registry (Sprint 5) ---");

    liz::ModelRegistry registry;
    registry.register_model("liz_upscaler_v1",     liz::ModelType::Upscaler,     "1.0.0");
    registry.register_model("liz_interpolator_v1", liz::ModelType::Interpolator, "1.0.0");
    registry.register_model("liz_denoiser_v1",     liz::ModelType::Denoiser,     "1.0.0");

    {
        auto all = registry.get_model_list();
        std::ostringstream oss;
        oss << "ModelRegistry: " << registry.size() << " models registered: ";
        for (std::size_t i = 0; i < all.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << "'" << all[i] << "'";
        }
        LIZ_INFO(oss.str());
    }

    {
        auto upscalers = registry.get_models_by_type(liz::ModelType::Upscaler);
        std::ostringstream oss;
        oss << "ModelRegistry: upscaler models: ";
        for (std::size_t i = 0; i < upscalers.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << "'" << upscalers[i] << "'";
        }
        LIZ_INFO(oss.str());
    }

    std::cout << std::endl;

    // -- 7. Inference Engine (Sprint 5 — model-based) --------------------------
    LIZ_INFO("--- Inference Engine (Sprint 5) ---");

    liz::InferenceEngine inf_engine;
    if (!inf_engine.initialize(gpu_ctx)) {
        LIZ_ERROR("Failed to initialize Inference Engine");
        return 1;
    }

    // Load models into the inference engine.
    inf_engine.load_model("liz_upscaler_v1");
    inf_engine.load_model("liz_denoiser_v1");

    {
        auto names = inf_engine.loaded_model_names();
        std::ostringstream oss;
        oss << "InferenceEngine: " << names.size() << " model(s) loaded: ";
        for (std::size_t i = 0; i < names.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << "'" << names[i] << "'";
        }
        LIZ_INFO(oss.str());
    }

    std::cout << std::endl;

    // Run inference pipeline on decoded frames.
    if (decode_result.success && !decode_result.frames.empty()) {
        auto inf_stats = inf_engine.run_pipeline(engine, decode_result.frames);

        std::cout << std::endl;

        // -- 8. Direct inference demo (single frame) ----------------------------
        LIZ_INFO("--- Direct Inference Demo (single frame) ---");

        liz::VideoFrame test_frame(99, 640, 480, 42.0,
                              std::vector<std::uint8_t>(640 * 480 * 3, 128));

        auto enhanced = inf_engine.infer(test_frame);

        std::ostringstream oss;
        oss << "Direct inference: input " << test_frame.width()
            << "x" << test_frame.height()
            << " -> output " << enhanced.width()
            << "x" << enhanced.height();
        LIZ_INFO(oss.str());

        if (enhanced.has_meta("model")) {
            std::ostringstream oss2;
            oss2 << "  model used: " << enhanced.get_meta("model");
            LIZ_INFO(oss2.str());
        }
        if (enhanced.has_meta("enhancement")) {
            std::ostringstream oss3;
            oss3 << "  enhancement: " << enhanced.get_meta("enhancement");
            LIZ_INFO(oss3.str());
        }

        std::cout << std::endl;
    }

    // -- 9. Full Pipeline Summary (Sprint 5) ------------------------------------
    {
        std::ostringstream oss;
        oss << "============================================" << std::endl
            << "  FULL PIPELINE SUMMARY (Sprint 5)" << std::endl
            << "============================================" << std::endl
            << "  GPU Backend:      " << gpu_ctx.backend_name()
            << " (" << gpu_ctx.device_info() << ")" << std::endl
            << "  Video Streaming:  " << vp_stats.frames_received << " frames, "
            << "gpu=" << (vp_stats.gpu_routed ? "yes" : "no") << ", "
            << vp_stats.execution_time_ms << " ms" << std::endl
            << "  AI Processors:    " << ai_stats.frames_input << " frames, "
            << ai_stats.tasks_completed << "/" << ai_stats.tasks_submitted << " tasks, "
            << ai_stats.processors_used << " processors, "
            << ai_stats.total_time_ms << " ms" << std::endl
            << "  Model Registry:   " << registry.size() << " models" << std::endl
            << "  Inference Engine: "
            << inf_engine.model_count() << " models, "
            << inf_engine.gpu_backend_name() << " backend" << std::endl
            << "  Flow: Video -> GPU -> AI Processors" << std::endl
            << "     -> Model Layer -> Inference Engine" << std::endl
            << "     -> GPU Routing -> Task System" << std::endl
            << "     -> Scheduler -> Enhanced Output";
        LIZ_INFO(oss.str());
    }

    std::cout << std::endl;

    // -- 10. Cleanup -----------------------------------------------------------
    inf_engine.shutdown();
    ai_pipeline.clear_processors();
    gpu_ctx.shutdown();
    engine.shutdown();

    std::cout << std::endl;
    return 0;
}