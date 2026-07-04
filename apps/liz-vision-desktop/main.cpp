// LIZ Vision — Desktop Application Entry Point
// Sprint 1+2+3+4+5+6+7 Demo: Engine, video pipeline, AI pipeline,
//   GPU routing, streaming, inference layer, performance layer,
//   GPU execution layer (simple stable version — no internal concurrency).

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
#include "engine/gpu/compute/GPUComputeEngine.h"
#include "engine/gpu/compute/GPUCommand.h"
#include "engine/performance/ThreadPool.h"
#include "engine/performance/FrameQueue.h"
#include "engine/performance/TaskExecutor.h"
#include "engine/performance/BatchProcessor.h"
#include "engine/performance/PerformanceManager.h"

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

    // -- 3. GPU Context (Sprint 4) ---------------------------------------------
    LIZ_INFO("--- GPU Layer (Sprint 4) ---");

    liz::GPUContext gpu_ctx;
    if (!gpu_ctx.initialize()) {
        LIZ_ERROR("Failed to initialize GPU context");
        return 1;
    }
    gpu_ctx.log_routing_decision();

    std::cout << std::endl;

    // -- 4. ThreadPool (Sprint 6) ---------------------------------------------
    LIZ_INFO("--- Performance Layer (Sprint 6) ---");

    liz::ThreadPool pool(4);
    pool.start();

    liz::PerformanceManager perf_mgr;

    std::cout << std::endl;

    // -- 5. GPU Compute Engine (Sprint 7 — STABLE) ----------------------------
    LIZ_INFO("--- GPU Execution Layer (Sprint 7 — stable) ---");

    liz::GPUComputeEngine gpu_compute;
    if (!gpu_compute.initialize(512)) {  // 512 MB VRAM
        LIZ_ERROR("Failed to initialize GPU Compute Engine");
        return 1;
    }

    // Log initial memory pool state.
    gpu_compute.memory_pool().log_summary();

    std::cout << std::endl;

    // -- 6. Video Pipeline (streaming) ----------------------------------------
    LIZ_INFO("--- Video Input Layer (streaming) ---");

    liz::VideoPipeline video_pipeline;
    auto vp_stats = video_pipeline.run_streaming(engine, "input_video.mp4", gpu_ctx);

    std::cout << std::endl;

    // -- 7. Decode frames for GPU pipeline demo --------------------------------
    liz::VideoDecoder decoder;
    liz::DecodeRequest req;
    req.source_path   = "input_video.mp4";
    req.target_width  = 320;
    req.target_height = 240;
    req.fps           = 24.0;
    req.max_frames    = 6;

    auto decode_result = decoder.decode(req);

    std::cout << std::endl;

    // -- 8. GPU Command Pipeline Demo (Sprint 7) ------------------------------
    LIZ_INFO("--- GPU Command Pipeline Demo ---");

    if (decode_result.success && !decode_result.frames.empty()) {

        // 8a. Upload all frames to GPU (submit only).
        LIZ_INFO("Phase 1: Submitting upload commands...");
        for (const auto& frame : decode_result.frames) {
            std::ostringstream name;
            name << "upload_frame_" << frame.frame_id();
            gpu_compute.submit(
                std::make_shared<liz::UploadFrameCommand>(
                    name.str(), frame.data_size_bytes()));
            perf_mgr.record_frame_produced();
        }

        // Process all pending commands.
        gpu_compute.process_all();

        // 8b. Run inference commands.
        LIZ_INFO("Phase 2: Submitting inference commands...");
        for (std::size_t i = 0; i < decode_result.frames.size(); ++i) {
            std::ostringstream name;
            name << "inference_frame_" << i;
            gpu_compute.submit(
                std::make_shared<liz::RunInferenceCommand>(
                    name.str(), "liz_upscaler_v1", "upscale_2x"));
        }

        gpu_compute.process_all();

        // 8c. Copy buffer commands (download simulation).
        LIZ_INFO("Phase 3: Submitting copy buffer commands...");
        for (std::size_t i = 0; i < decode_result.frames.size(); ++i) {
            std::ostringstream name;
            name << "copy_buffer_" << i;
            gpu_compute.submit(
                std::make_shared<liz::CopyBufferCommand>(
                    name.str(), static_cast<std::uint64_t>(i),
                    decode_result.frames[i].data_size_bytes()));
            perf_mgr.record_frame_consumed();
        }

        gpu_compute.process_all();

        std::cout << std::endl;

        // 8d. Verify FIFO order with explicit process_next().
        LIZ_INFO("Phase 4: Verifying FIFO with process_next()...");
        gpu_compute.submit(
            std::make_shared<liz::UploadFrameCommand>("fifo_upload_1", 1024));
        gpu_compute.submit(
            std::make_shared<liz::RunInferenceCommand>("fifo_infer_1", "model_a", "op_x"));
        gpu_compute.submit(
            std::make_shared<liz::CopyBufferCommand>("fifo_copy_1", 1, 2048));

        // Process one by one to verify FIFO.
        while (gpu_compute.process_next()) {
            // each call pops the front command
        }

        std::cout << std::endl;

        // 8e. Memory pool demo.
        LIZ_INFO("Phase 5: Memory pool allocation demo...");
        auto buf1 = gpu_compute.memory_pool().allocate(1024 * 1024);  // 1 MB
        auto buf2 = gpu_compute.memory_pool().allocate(2 * 1024 * 1024);  // 2 MB
        gpu_compute.memory_pool().log_summary();

        gpu_compute.memory_pool().free(buf1.id);
        gpu_compute.memory_pool().log_summary();

        gpu_compute.memory_pool().free(buf2.id);
        gpu_compute.memory_pool().log_summary();
    }

    std::cout << std::endl;

    // -- 9. AI Pipeline (Sprint 3 — preserved) --------------------------------
    LIZ_INFO("--- AI Processing Layer (Sprint 3) ---");

    liz::AIPipeline ai_pipeline;
    ai_pipeline.context().load_defaults();
    ai_pipeline.add_processor(std::make_shared<liz::DemoUpscalerProcessor>());
    ai_pipeline.add_processor(std::make_shared<liz::DemoInterpolatorProcessor>());

    if (decode_result.success) {
        ai_pipeline.run(engine, decode_result.frames);
    }

    std::cout << std::endl;

    // -- 10. Model Registry + Inference (Sprint 5 — preserved) ----------------
    LIZ_INFO("--- Model + Inference Layer (Sprint 5) ---");

    liz::ModelRegistry registry;
    registry.register_model("liz_upscaler_v1",     liz::ModelType::Upscaler,     "1.0.0");
    registry.register_model("liz_interpolator_v1", liz::ModelType::Interpolator, "1.0.0");
    registry.register_model("liz_denoiser_v1",     liz::ModelType::Denoiser,     "1.0.0");

    liz::InferenceEngine inf_engine;
    if (inf_engine.initialize(gpu_ctx)) {
        inf_engine.load_model("liz_upscaler_v1");
        if (decode_result.success) {
            inf_engine.run_pipeline(engine, decode_result.frames);
        }
        inf_engine.shutdown();
    }

    std::cout << std::endl;

    // -- 11. BatchProcessor (Sprint 6 — preserved) ---------------------------
    LIZ_INFO("--- BatchProcessor Demo (parallel) ---");

    if (decode_result.success) {
        liz::TaskExecutor executor(pool);
        liz::BatchProcessor batch_proc(executor, 3);

        batch_proc.process(decode_result.frames,
            [](std::size_t idx, const std::vector<liz::VideoFrame>& batch) -> bool {
                std::ostringstream oss;
                oss << "  Batch #" << idx << ": "
                    << batch.size() << " frames via GPU compute path";
                LIZ_INFO(oss.str());
                return true;
            }
        );
    }

    std::cout << std::endl;

    // -- 12. Performance Summary -----------------------------------------------
    perf_mgr.log_summary();

    std::cout << std::endl;

    // -- 13. Full Pipeline Summary (Sprint 7) ----------------------------------
    {
        std::ostringstream oss;
        oss << "============================================" << std::endl
            << "  FULL PIPELINE SUMMARY (Sprint 7)" << std::endl
            << "============================================" << std::endl
            << "  GPU Backend:       " << gpu_ctx.backend_name()
            << " (" << gpu_ctx.device_info() << ")" << std::endl
            << "  GPU Compute:       "
            << gpu_compute.commands_processed() << " commands processed" << std::endl
            << "  GPU Queue (FIFO):  simple, no internal concurrency" << std::endl
            << "  VRAM Pool:         "
            << (gpu_compute.memory_pool().used_memory() / 1024) << "KB / "
            << (gpu_compute.memory_pool().total_memory() / (1024 * 1024)) << "MB" << std::endl
            << "  Video Streaming:   " << vp_stats.frames_received << " frames" << std::endl
            << "  ThreadPool:        " << pool.worker_count() << " workers" << std::endl
            << "  Performance:       fps=" << perf_mgr.effective_fps() << std::endl
            << "  Flow: CPU Scheduler + ThreadPool" << std::endl
            << "     -> GPU Command Queue (FIFO)" << std::endl
            << "     -> GPU Memory Pool (512 MB)" << std::endl
            << "     -> AI Inference Engine" << std::endl
            << "     -> Video Enhancement Output";
        LIZ_INFO(oss.str());
    }

    std::cout << std::endl;

    // -- 14. Cleanup -----------------------------------------------------------
    gpu_compute.shutdown();
    pool.shutdown();
    ai_pipeline.clear_processors();
    gpu_ctx.shutdown();
    engine.shutdown();

    std::cout << std::endl;
    return 0;
}