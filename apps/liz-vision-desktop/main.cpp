// LIZ Vision — Desktop Application Entry Point
// Sprint 1+2 Demo: Engine, plugins, task system, and video pipeline.

#include "engine/core/Engine.h"
#include "engine/core/Logger.h"
#include "engine/plugins/PluginInterface.h"
#include "engine/video/VideoPipeline.h"

#include <iostream>
#include <memory>
#include <sstream>

// ── Dummy Plugin ──────────────────────────────────────────────────────────────
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

    // ── 1. Engine initialization (Sprint 1) ──────────────────────────────────
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

    // ── 3. Video Pipeline (Sprint 2) ─────────────────────────────────────────
    liz::VideoPipeline pipeline;

    liz::DecodeRequest req;
    req.source_path    = "sample_video.mp4";  // simulated — not read from disk
    req.target_width   = 320;                  // small for fast demo
    req.target_height  = 240;
    req.fps            = 24.0;
    req.max_frames     = 8;

    auto stats = pipeline.run(engine, req);

    std::cout << std::endl;

    // ── 4. Pipeline summary ──────────────────────────────────────────────────
    {
        std::ostringstream oss;
        oss << "Final stats: " << stats.frames_received << " frames, "
            << stats.tasks_completed << "/" << stats.tasks_submitted << " tasks, "
            << stats.tasks_failed << " failed, "
            << "decode=" << stats.decode_time_ms << "ms, "
            << "exec=" << stats.execution_time_ms << "ms";
        LIZ_INFO(oss.str());
    }

    std::cout << std::endl;

    // ── 5. Shutdown ───────────────────────────────────────────────────────────
    engine.shutdown();

    std::cout << std::endl;
    return 0;
}