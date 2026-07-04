// LIZ Vision — Desktop Application Entry Point
// Sprint 1+2+3 Demo: Engine, plugins, video pipeline, AI pipeline.

#include "engine/core/Engine.h"
#include "engine/core/Logger.h"
#include "engine/plugins/PluginInterface.h"
#include "engine/video/VideoPipeline.h"
#include "engine/ai/AIPipeline.h"
#include "engine/ai/DemoProcessors.h"

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

    // ── 2. Register plugins (Sprint 1) ───────────────────────────────────────
    engine.register_plugin(std::make_shared<DummyPlugin>("Demo Upscaler", "upscaler"));
    engine.register_plugin(std::make_shared<DummyPlugin>("Demo Detector", "detection"));

    std::cout << std::endl;

    // ── 3. Video Pipeline — decode simulated frames (Sprint 2) ───────────────
    liz::VideoPipeline video_pipeline;

    liz::DecodeRequest req;
    req.source_path    = "sample_video.mp4";
    req.target_width   = 320;
    req.target_height  = 240;
    req.fps            = 24.0;
    req.max_frames     = 6;

    auto video_stats = video_pipeline.run(engine, req);

    std::cout << std::endl;

    // ── 4. AI Pipeline — process frames through AI processors (Sprint 3) ────
    LIZ_INFO("--- AI Processing Layer ---");

    liz::AIPipeline ai_pipeline;
    ai_pipeline.context().load_defaults();

    // Register demo AI processors
    auto upscaler     = std::make_shared<liz::DemoUpscalerProcessor>();
    auto interpolator = std::make_shared<liz::DemoInterpolatorProcessor>();

    ai_pipeline.add_processor(upscaler);
    ai_pipeline.add_processor(interpolator);

    std::cout << std::endl;

    // Run AI pipeline with the same frames (simulated decode again for demo)
    liz::VideoDecoder decoder;
    auto decode_result = decoder.decode(req);

    if (decode_result.success) {
        auto ai_stats = ai_pipeline.run(engine, decode_result.frames);

        std::cout << std::endl;

        // ── 5. Full pipeline summary ──────────────────────────────────────────
        std::ostringstream oss;
        oss << "=== FULL PIPELINE SUMMARY ===" << std::endl
            << "  Video decode:   " << video_stats.frames_received << " frames, "
            << video_stats.tasks_completed << " tasks, "
            << video_stats.decode_time_ms << " ms decode" << std::endl
            << "  AI processing:  " << ai_stats.frames_input << " frames, "
            << ai_stats.tasks_completed << "/" << ai_stats.tasks_submitted << " tasks, "
            << ai_stats.processors_used << " processors, "
            << ai_stats.total_time_ms << " ms" << std::endl
            << "  Total flow: Frame → AI Processing → Task System → Scheduler → Output";
        LIZ_INFO(oss.str());
    }

    std::cout << std::endl;

    // ── 6. Shutdown ─────────────────────────────────────────────────────────
    ai_pipeline.clear_processors();
    engine.shutdown();

    std::cout << std::endl;
    return 0;
}