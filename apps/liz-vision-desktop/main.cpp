// LIZ Vision — Desktop Application Entry Point
// Sprint 1+2+3+4+5+6 Demo: Engine, video pipeline, AI pipeline,
//   GPU routing, streaming, inference layer, performance layer.

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

    // -- 3. GPU Context --------------------------------------------------------
    LIZ_INFO("--- GPU Layer ---");

    liz::GPUContext gpu_ctx;
    if (!gpu_ctx.initialize()) {
        LIZ_ERROR("Failed to initialize GPU context");
        return 1;
    }
    gpu_ctx.log_routing_decision();

    std::cout << std::endl;

    // -- 4. Performance Layer (Sprint 6) ----------------------------------------
    LIZ_INFO("--- Performance Layer (Sprint 6) ---");

    liz::PerformanceManager perf_mgr;

    // -- 4a. ThreadPool --------------------------------------------------------
    liz::ThreadPool pool(4);  // 4 worker threads
    pool.start();

    {
        auto ps = pool.stats();
        std::ostringstream oss;
        oss << "ThreadPool: " << ps.total_workers << " workers ready";
        LIZ_INFO(oss.str());
    }

    // -- 4b. TaskExecutor ------------------------------------------------------
    liz::TaskExecutor executor(pool);

    // -- 4c. FrameQueue (backpressure) ----------------------------------------
    liz::FrameQueue frame_queue(8);  // capacity 8

    std::cout << std::endl;

    // -- 5. Video Pipeline (Sprint 4 — streaming) ------------------------------
    LIZ_INFO("--- Video Input Layer (streaming + GPU) ---");

    liz::VideoPipeline video_pipeline;

    auto vp_stats = video_pipeline.run_streaming(
        engine,
        "input_video.mp4",
        gpu_ctx
    );

    std::cout << std::endl;

    // -- 6. AI Pipeline (Sprint 3 — processor-based) ---------------------------
    LIZ_INFO("--- AI Processing Layer (Sprint 3 processors) ---");

    liz::AIPipeline ai_pipeline;
    ai_pipeline.context().load_defaults();
    ai_pipeline.add_processor(std::make_shared<liz::DemoUpscalerProcessor>());
    ai_pipeline.add_processor(std::make_shared<liz::DemoInterpolatorProcessor>());

    // Decode frames for processing.
    liz::VideoDecoder decoder;
    liz::DecodeRequest req;
    req.source_path   = "input_video.mp4";
    req.target_width  = 320;
    req.target_height = 240;
    req.fps           = 24.0;
    req.max_frames    = 8;

    auto decode_result = decoder.decode(req);
    liz::AIPipelineStats ai_stats{};
    if (decode_result.success) {
        ai_stats = ai_pipeline.run(engine, decode_result.frames);
    }

    std::cout << std::endl;

    // -- 7. Model Registry + Inference Engine (Sprint 5) -----------------------
    LIZ_INFO("--- Model + Inference Layer (Sprint 5) ---");

    liz::ModelRegistry registry;
    registry.register_model("liz_upscaler_v1",     liz::ModelType::Upscaler,     "1.0.0");
    registry.register_model("liz_interpolator_v1", liz::ModelType::Interpolator, "1.0.0");
    registry.register_model("liz_denoiser_v1",     liz::ModelType::Denoiser,     "1.0.0");

    liz::InferenceEngine inf_engine;
    if (!inf_engine.initialize(gpu_ctx)) {
        LIZ_ERROR("Failed to initialize Inference Engine");
        return 1;
    }

    inf_engine.load_model("liz_upscaler_v1");
    inf_engine.load_model("liz_denoiser_v1");

    {
        auto names = inf_engine.loaded_model_names();
        std::ostringstream oss;
        oss << "InferenceEngine: " << names.size() << " model(s) loaded";
        LIZ_INFO(oss.str());
    }

    std::cout << std::endl;

    // -- 8. FrameQueue Demo: producer-consumer with backpressure ---------------
    LIZ_INFO("--- FrameQueue Demo (backpressure) ---");

    if (decode_result.success) {
        // Producer: push frames into the queue.
        for (const auto& frame : decode_result.frames) {
            perf_mgr.record_frame_produced();
            frame_queue.push(frame);

            std::ostringstream oss;
            oss << "FrameQueue: produced Frame #" << frame.frame_id()
                << " [queue=" << frame_queue.size()
                << "/" << frame_queue.capacity() << "]";
            LIZ_INFO(oss.str());
        }

        LIZ_INFO("FrameQueue: all frames pushed, closing for consumers...");
        frame_queue.close();

        // Consumer: drain the queue.
        std::size_t consumed = 0;
        while (auto frame = frame_queue.pop()) {
            ++consumed;
            perf_mgr.record_frame_consumed();

            std::ostringstream oss;
            oss << "FrameQueue: consumed Frame #" << frame->frame_id()
                << " [" << frame->width() << "x" << frame->height() << "]";
            LIZ_INFO(oss.str());
        }

        {
            std::ostringstream oss;
            oss << "FrameQueue: " << consumed << " frames consumed, "
                << frame_queue.backpressure_events() << " backpressure events";
            LIZ_INFO(oss.str());
        }
    }

    std::cout << std::endl;

    // -- 9. BatchProcessor + ThreadPool Demo -----------------------------------
    LIZ_INFO("--- BatchProcessor Demo (parallel) ---");

    if (decode_result.success && !decode_result.frames.empty()) {
        liz::BatchProcessor batch_proc(executor, 4);  // batch size 4

        batch_proc.process(decode_result.frames,
            [&perf_mgr](std::size_t batch_idx,
                        const std::vector<liz::VideoFrame>& batch) -> bool {

                auto start = std::chrono::steady_clock::now();

                // Simulate GPU-optimized batch inference.
                for (const auto& f : batch) {
                    perf_mgr.record_latency(
                        "batch_" + std::to_string(batch_idx) +
                        "_frame_" + std::to_string(f.frame_id()),
                        0.5);  // simulated 0.5ms per frame
                }

                auto end = std::chrono::steady_clock::now();
                double elapsed =
                    std::chrono::duration<double, std::milli>(end - start).count();

                perf_mgr.record_batch_time(elapsed);

                std::ostringstream oss;
                oss << "  Batch #" << batch_idx << ": "
                    << batch.size() << " frames processed in "
                    << elapsed << " ms (parallel via ThreadPool)";
                LIZ_INFO(oss.str());

                return true;
            }
        );

        auto bs = batch_proc.last_stats();

        {
            std::ostringstream oss;
            oss << "BatchProcessor stats: "
                << bs.frames_processed << " frames, "
                << bs.total_batches << " batches, "
                << bs.total_time_ms << " ms total, "
                << bs.avg_batch_ms << " ms avg/batch";
            LIZ_INFO(oss.str());
        }
    }

    std::cout << std::endl;

    // -- 10. Async Task Execution Demo -----------------------------------------
    LIZ_INFO("--- Async Task Execution Demo ---");

    executor.reset_stats();

    for (std::size_t i = 0; i < 12; ++i) {
        auto idx = i;
        executor.submit(
            "AsyncJob_" + std::to_string(i),
            [idx, &perf_mgr]() -> bool {
                // Simulate work.
                auto start = std::chrono::steady_clock::now();
                std::this_thread::sleep_for(std::chrono::milliseconds(2));
                auto end = std::chrono::steady_clock::now();

                double elapsed =
                    std::chrono::duration<double, std::milli>(end - start).count();

                perf_mgr.record_latency("async_job_" + std::to_string(idx),
                                         elapsed);

                std::ostringstream oss;
                oss << "  AsyncJob_" << idx
                    << " completed in " << elapsed << " ms";
                LIZ_INFO(oss.str());

                return true;
            }
        );
    }

    executor.wait_all();

    {
        auto es = executor.stats();
        std::ostringstream oss;
        oss << "TaskExecutor stats: "
            << es.tasks_completed << "/" << es.tasks_submitted << " tasks, "
            << es.tasks_failed << " failed, "
            << es.total_time_ms << " ms total, "
            << es.avg_time_ms << " ms avg";
        LIZ_INFO(oss.str());
    }

    std::cout << std::endl;

    // -- 11. ThreadPool stats --------------------------------------------------
    {
        auto ps = pool.stats();
        std::ostringstream oss;
        oss << "ThreadPool final: "
            << ps.tasks_completed << " completed, "
            << ps.tasks_failed << " failed, "
            << ps.total_workers << " workers";
        LIZ_INFO(oss.str());
    }

    std::cout << std::endl;

    // -- 12. Performance Summary -----------------------------------------------
    LIZ_INFO("--- Performance Summary (Sprint 6) ---");
    perf_mgr.log_summary();

    std::cout << std::endl;

    // -- 13. Full Pipeline Summary (Sprint 6) ----------------------------------
    {
        std::ostringstream oss;
        oss << "============================================" << std::endl
            << "  FULL PIPELINE SUMMARY (Sprint 6)" << std::endl
            << "============================================" << std::endl
            << "  GPU Backend:       " << gpu_ctx.backend_name()
            << " (" << gpu_ctx.device_info() << ")" << std::endl
            << "  Video Streaming:   " << vp_stats.frames_received << " frames, "
            << "gpu=" << (vp_stats.gpu_routed ? "yes" : "no") << ", "
            << vp_stats.execution_time_ms << " ms" << std::endl
            << "  AI Processors:     " << ai_stats.frames_input << " frames, "
            << ai_stats.tasks_completed << "/" << ai_stats.tasks_submitted << " tasks" << std::endl
            << "  Model Registry:    " << registry.size() << " models" << std::endl
            << "  Inference Engine:  "
            << inf_engine.model_count() << " models loaded" << std::endl
            << "  ThreadPool:        " << pool.worker_count() << " workers" << std::endl
            << "  FrameQueue:        " << frame_queue.total_pushed() << " pushed, "
            << frame_queue.total_popped() << " popped, "
            << frame_queue.backpressure_events() << " bp events" << std::endl
            << "  Performance:       "
            << "fps=" << perf_mgr.effective_fps() << " "
            << "avg_lat=" << perf_mgr.avg_frame_latency_ms() << "ms" << std::endl
            << "  Flow: Video -> FrameQueue(backpressure)" << std::endl
            << "     -> BatchProcessor -> ThreadPool" << std::endl
            << "     -> GPU Routing -> Inference Engine" << std::endl
            << "     -> PerformanceManager -> Output";
        LIZ_INFO(oss.str());
    }

    std::cout << std::endl;

    // -- 14. Cleanup -----------------------------------------------------------
    pool.shutdown();
    inf_engine.shutdown();
    ai_pipeline.clear_processors();
    gpu_ctx.shutdown();
    engine.shutdown();

    std::cout << std::endl;
    return 0;
}