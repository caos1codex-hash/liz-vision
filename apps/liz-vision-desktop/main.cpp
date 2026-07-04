// LIZ Vision — Desktop Application Entry Point
// Sprint 1+2+3+4 Demo: Engine, video pipeline, AI pipeline, GPU routing, streaming.

#include "engine/core/Engine.h"
#include "engine/core/Logger.h"
#include "engine/plugins/PluginInterface.h"
#include "engine/video/VideoPipeline.h"
#include "engine/ai/AIPipeline.h"
#include "engine/ai/DemoProcessors.h"
#include "engine/gpu/GPUContext.h"

#include <iostream>
#include <memory>
#include <sstream>

// ── Dummy Plugin (Sprint 1) ──────────────────────────────────────────────────
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

// ── Main ──────────────────────────────────────────────────────────────────────
int main() {
    std::cout << std::endl;

    // ── 1. Engine initialization ─────────────────────────────────────────────
    liz::Engine engine;
    if (!engine.initialize()) {
        std::cerr << "Failed to initialize LIZ Vision Engine" << std::endl;
        return 1;
    }

    std::cout << std::endl;

    // ── 2. Register plugins ──────────────────────────────────────────────────
    engine.register_plugin(std::make_shared<DummyPlugin>("Demo Upscaler", "upscaler"));
    engine.register_plugin(std::make_shared<DummyPlugin>("Demo Detector", "detection"));

    std::cout << std::endl;

    // ── 3. GPU Context — probe and initialize ────────────────────────────────
    LIZ_INFO("--- GPU Layer ---");

    liz::GPUContext gpu_ctx;
    if (!gpu_ctx.initialize()) {
        LIZ_ERROR("Failed to initialize GPU context");
        return 1;
    }
    gpu_ctx.log_routing_decision();

    std::cout << std::endl;

    // ── 4. Streaming Video Pipeline with GPU routing ─────────────────────────
    LIZ_INFO("--- Video Input Layer (streaming + GPU) ---");

    liz::VideoPipeline video_pipeline;

    auto vp_stats = video_pipeline.run_streaming(
        engine,
        "input_video.mp4",    // simulated file
        gpu_ctx
    );

    std::cout << std::endl;

    // ── 5. AI Pipeline on the same frames ────────────────────────────────────
    LIZ_INFO("--- AI Processing Layer ---");

    liz::AIPipeline ai_pipeline;
    ai_pipeline.context().load_defaults();
    ai_pipeline.add_processor(std::make_shared<liz::DemoUpscalerProcessor>());
    ai_pipeline.add_processor(std::make_shared<liz::DemoInterpolatorProcessor>());

    std::cout << std::endl;

    // Re-decode frames for AI pipeline demo.
    liz::VideoDecoder decoder;
    liz::DecodeRequest req;
    req.source_path   = "input_video.mp4";
    req.target_width  = 320;
    req.target_height = 240;
    req.fps           = 24.0;
    req.max_frames    = 5;

    auto decode_result = decoder.decode(req);
    if (decode_result.success) {
        auto ai_stats = ai_pipeline.run(engine, decode_result.frames);

        std::cout << std::endl;

        // ── 6. Full pipeline summary ──────────────────────────────────────────
        std::ostringstream oss;
        oss << "============================================" << std::endl
            << "  FULL PIPELINE SUMMARY (Sprint 4)" << std::endl
            << "============================================" << std::endl
            << "  GPU Backend:    " << gpu_ctx.backend_name()
            << " (" << gpu_ctx.device_info() << ")" << std::endl
            << "  Video Streaming: " << vp_stats.frames_received << " frames, "
            << "gpu=" << (vp_stats.gpu_routed ? "yes" : "no") << ", "
            << vp_stats.execution_time_ms << " ms" << std::endl
            << "  AI Processing:  " << ai_stats.frames_input << " frames, "
            << ai_stats.tasks_completed << "/" << ai_stats.tasks_submitted << " tasks, "
            << ai_stats.processors_used << " processors, "
            << ai_stats.total_time_ms << " ms" << std::endl
            << "  Flow: Real Video Input -> GPU Routing -> AI Pipeline"
            << " -> Task System -> Scheduler -> Output";
        LIZ_INFO(oss.str());
    }

    std::cout << std::endl;

    // ── 7. Cleanup ──────────────────────────────────────────────────────────
    ai_pipeline.clear_processors();
    gpu_ctx.shutdown();
    engine.shutdown();

    std::cout << std::endl;
    return 0;
}