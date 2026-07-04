// LIZ Vision — Desktop Application Entry Point
// Sprint 1+2+3+4+5+6+7+8+9 Demo: Engine, video pipeline, AI pipeline,
//   GPU routing, streaming, inference layer, performance layer,
//   GPU execution layer (FIFO), tensor + batch processing layer,
//   resource management foundation.

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
#include "engine/ai/tensor/Tensor.h"
#include "engine/ai/tensor/TensorShape.h"
#include "engine/gpu/GPUContext.h"
#include "engine/gpu/compute/GPUComputeEngine.h"
#include "engine/gpu/compute/GPUCommand.h"
#include "engine/gpu/batch/Batch.h"
#include "engine/gpu/batch/BatchProcessorGPU.h"
#include "engine/resources/ResourceManager.h"
#include "engine/resources/ResourceHandle.h"
#include "engine/runtime/Runtime.h"
#include "engine/events/EventBus.h"
#include "engine/events/Event.h"
#include "engine/events/EventListener.h"
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

// -- Event Bus Demo Listeners (Sprint 11) ------------------------------------
class VideoListener : public liz::EventListener {
public:
    std::string_view name() const override { return "VideoListener"; }
    void on_event(const liz::Event& event) override {
        if (event.type() == liz::EventType::EngineStarted ||
            event.type() == liz::EventType::EngineStopped ||
            event.type() == liz::EventType::FrameDecoded ||
            event.type() == liz::EventType::InferenceFinished) {
            std::ostringstream oss;
            oss << "  [VideoListener] received: " << event.info();
            LIZ_INFO(oss.str());
        }
    }
};

class GPUListener : public liz::EventListener {
public:
    std::string_view name() const override { return "GPUListener"; }
    void on_event(const liz::Event& event) override {
        if (event.type() == liz::EventType::EngineStarted ||
            event.type() == liz::EventType::EngineStopped ||
            event.type() == liz::EventType::GPUCommandSubmitted ||
            event.type() == liz::EventType::GPUCommandCompleted ||
            event.type() == liz::EventType::InferenceStarted ||
            event.type() == liz::EventType::InferenceFinished) {
            std::ostringstream oss;
            oss << "  [GPUListener] received: " << event.info();
            LIZ_INFO(oss.str());
        }
    }
};

class DiagnosticListener : public liz::EventListener {
public:
    std::string_view name() const override { return "DiagnosticListener"; }
    void on_event(const liz::Event& event) override {
        std::ostringstream oss;
        oss << "  [DiagnosticListener] received: " << event.info();
        LIZ_INFO(oss.str());
    }
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
    if (!gpu_compute.initialize(512)) {
        LIZ_ERROR("Failed to initialize GPU Compute Engine");
        return 1;
    }

    gpu_compute.memory_pool().log_summary();

    std::cout << std::endl;

    // -- 6. Video Pipeline (streaming) ----------------------------------------
    LIZ_INFO("--- Video Input Layer (streaming) ---");

    liz::VideoPipeline video_pipeline;
    auto vp_stats = video_pipeline.run_streaming(engine, "input_video.mp4", gpu_ctx);

    std::cout << std::endl;

    // -- 7. Decode frames ------------------------------------------------------
    liz::VideoDecoder decoder;
    liz::DecodeRequest req;
    req.source_path   = "input_video.mp4";
    req.target_width  = 320;
    req.target_height = 240;
    req.fps           = 24.0;
    req.max_frames    = 8;

    auto decode_result = decoder.decode(req);

    std::cout << std::endl;

    // -- 8. GPU Command Pipeline Demo (Sprint 7 — FIFO preserved) --------------
    LIZ_INFO("--- GPU Command Pipeline Demo (Sprint 7 FIFO) ---");

    if (decode_result.success && !decode_result.frames.empty()) {
        LIZ_INFO("Submitting upload + inference + copy commands...");
        for (const auto& frame : decode_result.frames) {
            std::ostringstream name;
            name << "upload_frame_" << frame.frame_id();
            gpu_compute.submit(
                std::make_shared<liz::UploadFrameCommand>(
                    name.str(), frame.data_size_bytes()));
            perf_mgr.record_frame_produced();
        }
        gpu_compute.process_all();

        for (std::size_t i = 0; i < decode_result.frames.size(); ++i) {
            std::ostringstream name;
            name << "inference_frame_" << i;
            gpu_compute.submit(
                std::make_shared<liz::RunInferenceCommand>(
                    name.str(), "liz_upscaler_v1", "upscale_2x"));
        }
        gpu_compute.process_all();

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
    }

    std::cout << std::endl;

    // -- 9. Tensor + Batch Layer (Sprint 8 — NEW) -----------------------------
    LIZ_INFO("--- Tensor + Batch Execution Layer (Sprint 8) ---");

    if (decode_result.success && !decode_result.frames.empty()) {

        // 9a. Tensor conversion demo.
        LIZ_INFO("Step 1: Frame -> Tensor conversion...");
        for (std::size_t i = 0; i < 3; ++i) {
            const auto& f = decode_result.frames[i];
            auto tensor = liz::Tensor::from_frame_data(
                f.width(), f.height(), f.data());
            tensor.set_type(liz::TensorType::ModelInput);

            std::ostringstream oss;
            oss << "  " << tensor.info();
            LIZ_INFO(oss.str());
        }

        // 9b. Tensor reshape demo.
        LIZ_INFO("Step 2: Tensor reshape demo...");
        liz::TensorShape shape{320, 240, 3};
        liz::Tensor t1(shape, liz::TensorType::Frame);
        {
            std::ostringstream oss;
            oss << "  Before: " << t1.info();
            LIZ_INFO(oss.str());
        }
        t1.reshape({640, 120, 3});
        {
            std::ostringstream oss;
            oss << "  After:  " << t1.info();
            LIZ_INFO(oss.str());
        }

        std::cout << std::endl;

        // 9c. Batch grouping demo (batch_size=4 for 8 frames -> 2 batches).
        LIZ_INFO("Step 3: Batch grouping (batch_size=4)...");
        liz::BatchProcessorGPU batch_proc_gpu(4);
        auto batches = batch_proc_gpu.process_frames(decode_result.frames);

        std::cout << std::endl;

        // 9d. GPU batch processing demo.
        LIZ_INFO("Step 4: GPU batch processing pipeline...");

        // Reset the GPU compute engine's memory pool for clean batch demo.
        liz::GPUComputeEngine batch_engine;
        batch_engine.initialize(512);

        for (const auto& batch : batches) {
            batch_engine.process_batch(batch, "liz_upscaler_v1", "upscale_2x");
        }

        std::cout << std::endl;

        // 9e. process_and_submit convenience demo (batch_size=2 -> 4 batches).
        LIZ_INFO("Step 5: BatchProcessorGPU.process_and_submit (batch_size=2)...");
        liz::BatchProcessorGPU small_batch_proc(2);

        liz::GPUComputeEngine small_batch_engine;
        small_batch_engine.initialize(512);

        small_batch_proc.process_and_submit(
            decode_result.frames, small_batch_engine,
            "liz_denoiser_v1", "denoise");

        std::cout << std::endl;

        // 9f. BatchProcessorGPU stats.
        auto bs = small_batch_proc.stats();
        {
            std::ostringstream oss;
            oss << "BatchProcessorGPU stats: "
                << bs.tensors_received << " tensors -> "
                << bs.batches_created << " batches (max="
                << bs.max_batch_size << "), "
                << bs.total_batches_processed << " submitted to GPU";
            LIZ_INFO(oss.str());
        }
    }

    std::cout << std::endl;

    // -- 10. AI Pipeline (Sprint 3 — preserved) --------------------------------
    LIZ_INFO("--- AI Processing Layer (Sprint 3) ---");

    liz::AIPipeline ai_pipeline;
    ai_pipeline.context().load_defaults();
    ai_pipeline.add_processor(std::make_shared<liz::DemoUpscalerProcessor>());
    ai_pipeline.add_processor(std::make_shared<liz::DemoInterpolatorProcessor>());

    if (decode_result.success) {
        ai_pipeline.run(engine, decode_result.frames);
    }

    std::cout << std::endl;

    // -- 11. Model Registry + Inference (Sprint 5 — preserved) ----------------
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

    // -- 12. BatchProcessor (Sprint 6 — preserved) ---------------------------
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

    // -- 13. Resource Management (Sprint 9 — NEW) ---------------------------
    LIZ_INFO("--- Resource Management Foundation (Sprint 9) ---");

    liz::ResourceManager res_mgr;

    // 13a. Create resources of various types.
    auto h_video  = res_mgr.create("input_video.mp4",         liz::ResourceType::Video,        1024 * 1024);
    auto h_frame0 = res_mgr.create("frame_0",                 liz::ResourceType::Frame,        230400);
    auto h_frame1 = res_mgr.create("frame_1",                 liz::ResourceType::Frame,        230400);
    auto h_tensor = res_mgr.create("upscale_tensor_0",        liz::ResourceType::Tensor,       921600);
    auto h_model  = res_mgr.create("liz_upscaler_v1",         liz::ResourceType::AIModel,      50 * 1024 * 1024);
    auto h_gpu    = res_mgr.create("gpu_buffer_pool",         liz::ResourceType::GPUBuffer,    512 * 1024 * 1024);
    auto h_plugin = res_mgr.create("Demo Upscaler",           liz::ResourceType::Plugin,       4096);
    auto h_config = res_mgr.create("engine_config",           liz::ResourceType::Configuration, 2048);
    auto h_proj   = res_mgr.create("liz_vision_project",      liz::ResourceType::Project,      0);
    auto h_temp   = res_mgr.create("temp_scratch_0",          liz::ResourceType::Temporary,    65536);

    std::cout << std::endl;

    // 13b. Query resources.
    LIZ_INFO("Resource queries:");

    {
        auto res = res_mgr.get(h_video);
        if (res) {
            std::ostringstream oss;
            oss << "  get(video handle): " << res->info();
            LIZ_INFO(oss.str());
        }
    }

    {
        auto found = res_mgr.find_by_name("frame_0");
        std::ostringstream oss;
        oss << "  find_by_name('frame_0'): " << found.info();
        LIZ_INFO(oss.str());
    }

    {
        std::ostringstream oss;
        oss << "  exists('frame_0'): " << (res_mgr.exists(h_frame0) ? "true" : "false");
        LIZ_INFO(oss.str());
    }

    std::cout << std::endl;

    // 13c. Reference counting.
    LIZ_INFO("Reference counting demo:");

    res_mgr.add_ref(h_frame0);
    res_mgr.add_ref(h_frame0);
    res_mgr.add_ref(h_frame1);

    {
        auto res = res_mgr.get(h_frame0);
        std::ostringstream oss;
        oss << "  frame_0 refs after +2: " << res->ref_count();
        LIZ_INFO(oss.str());
    }

    res_mgr.release(h_frame0);
    {
        auto res = res_mgr.get(h_frame0);
        std::ostringstream oss;
        oss << "  frame_0 refs after -1: " << res->ref_count();
        LIZ_INFO(oss.str());
    }

    std::cout << std::endl;

    // 13d. State changes.
    LIZ_INFO("State change demo:");
    {
        auto res = res_mgr.get(h_temp);
        res->set_state(liz::ResourceState::Unloaded);
        std::ostringstream oss;
        oss << "  temp_scratch_0: " << res->info();
        LIZ_INFO(oss.str());
    }

    std::cout << std::endl;

    // 13e. Statistics.
    LIZ_INFO("Statistics before cleanup:");
    res_mgr.log_statistics();

    std::cout << std::endl;

    // 13f. Destroy one resource.
    LIZ_INFO("Destroying one resource...");
    res_mgr.destroy(h_temp);

    std::cout << std::endl;

    // 13g. Statistics after destroy.
    LIZ_INFO("Statistics after destroy:");
    res_mgr.log_statistics();

    std::cout << std::endl;

    // 13h. Clear cache.
    LIZ_INFO("Clearing cache...");
    res_mgr.clear_cache();
    res_mgr.log_statistics();

    std::cout << std::endl;

    // -- 14. Runtime Architecture Foundation (Sprint 10 — NEW) ----------------
    LIZ_INFO("--- Runtime Architecture Foundation (Sprint 10) ---");

    {
        liz::Runtime runtime;

        // Show initial state.
        LIZ_INFO("Runtime created");
        runtime.status();

        std::cout << std::endl;

        // Initialize: Created -> Initializing -> Ready.
        if (!runtime.initialize()) {
            LIZ_ERROR("Runtime initialization failed");
            return 1;
        }

        std::cout << std::endl;
        runtime.status();

        std::cout << std::endl;

        // Run: Ready -> Running.
        LIZ_INFO("--- Run ---");
        runtime.run();
        runtime.status();

        std::cout << std::endl;

        // Pause: Running -> Paused.
        LIZ_INFO("--- Pause ---");
        runtime.pause();
        runtime.status();

        std::cout << std::endl;

        // Resume: Paused -> Running.
        LIZ_INFO("--- Resume ---");
        runtime.resume();
        runtime.status();

        std::cout << std::endl;

        // Invalid transition test: try to initialize again (should fail).
        LIZ_INFO("--- Invalid transition test ---");
        if (!runtime.initialize()) {
            LIZ_WARN("Correctly rejected: initialize() while Running");
        }

        // Invalid: try to go Running -> Ready.
        if (!runtime.pause()) {
            // Already tested pause from Running, so pause first then try run->ready
        }
        runtime.pause();
        runtime.status();

        // Try to initialize while Paused (should fail).
        if (!runtime.initialize()) {
            LIZ_WARN("Correctly rejected: initialize() while Paused");
        }

        std::cout << std::endl;

        // Resume before shutdown.
        runtime.resume();

        std::cout << std::endl;

        // Shutdown: Running -> Stopping -> Stopped.
        LIZ_INFO("--- Shutdown ---");
        runtime.shutdown();
        runtime.status();

        std::cout << std::endl;

        // Invalid: try to run after stopped (should fail).
        LIZ_INFO("--- Post-shutdown invalid transition test ---");
        if (!runtime.run()) {
            LIZ_WARN("Correctly rejected: run() after Stopped");
        }
    }

    std::cout << std::endl;

    // -- 15. Event Bus Foundation (Sprint 11 — NEW) --------------------------
    LIZ_INFO("--- Event Bus Foundation (Sprint 11) ---");

    {
        liz::EventBus bus;
        VideoListener video_listener;
        GPUListener gpu_listener;
        DiagnosticListener diagnostic_listener;

        // Subscribe all three listeners.
        bus.subscribe(&video_listener);
        bus.subscribe(&gpu_listener);
        bus.subscribe(&diagnostic_listener);

        std::cout << std::endl;

        // Publish events demonstrating selective reception.
        LIZ_INFO("Publishing EngineStarted...");
        bus.publish(liz::Event(liz::EventType::EngineStarted, "Runtime",
                               "Engine has started", liz::EventPriority::High));

        std::cout << std::endl;

        LIZ_INFO("Publishing FrameDecoded...");
        bus.publish(liz::Event(liz::EventType::FrameDecoded, "VideoPipeline",
                               "Frame 42 decoded (320x240)", liz::EventPriority::Normal));

        std::cout << std::endl;

        LIZ_INFO("Publishing TensorCreated...");
        bus.publish(liz::Event(liz::EventType::TensorCreated, "TensorLayer",
                               "Tensor upscaled_input created (3x320x240)", liz::EventPriority::Normal));

        std::cout << std::endl;

        LIZ_INFO("Publishing InferenceFinished...");
        bus.publish(liz::Event(liz::EventType::InferenceFinished, "InferenceEngine",
                               "Batch 1 inference completed (4 frames)", liz::EventPriority::High));

        std::cout << std::endl;

        LIZ_INFO("Publishing EngineStopped...");
        bus.publish(liz::Event(liz::EventType::EngineStopped, "Runtime",
                               "Engine has stopped", liz::EventPriority::Critical));

        std::cout << std::endl;

        // Unsubscribe diagnostic listener.
        LIZ_INFO("Unsubscribing DiagnosticListener...");
        bus.unsubscribe(&diagnostic_listener);

        std::cout << std::endl;

        // Publish one more event to show selective delivery after unsubscribe.
        LIZ_INFO("Publishing EngineStarted after unsubscribe...");
        bus.publish(liz::Event(liz::EventType::EngineStarted, "Runtime",
                               "Engine restarted (diagnostic gone)", liz::EventPriority::Normal));

        std::cout << std::endl;

        // Statistics.
        bus.log_statistics();
    }

    std::cout << std::endl;

    // -- 16. Performance Summary -----------------------------------------------
    perf_mgr.log_summary();

    std::cout << std::endl;

    // -- 17. Full Pipeline Summary (Sprint 11) ----------------------------------
    {
        std::ostringstream oss;
        oss << "============================================" << std::endl
            << "  FULL PIPELINE SUMMARY (Sprint 11)" << std::endl
            << "============================================" << std::endl
            << "  GPU Backend:       " << gpu_ctx.backend_name()
            << " (" << gpu_ctx.device_info() << ")" << std::endl
            << "  GPU Compute:       "
            << gpu_compute.commands_processed() << " FIFO commands" << std::endl
            << "  GPU Queue (FIFO):  simple, no internal concurrency" << std::endl
            << "  VRAM Pool:         "
            << (gpu_compute.memory_pool().used_memory() / 1024) << "KB / "
            << (gpu_compute.memory_pool().total_memory() / (1024 * 1024)) << "MB" << std::endl
            << "  Video Streaming:   " << vp_stats.frames_received << " frames" << std::endl
            << "  ThreadPool:        " << pool.worker_count() << " workers" << std::endl
            << "  Performance:       fps=" << perf_mgr.effective_fps() << std::endl
            << "  Flow: Frame Stream" << std::endl
            << "     -> Tensor Conversion Layer" << std::endl
            << "     -> Batch GPU Processor" << std::endl
            << "     -> GPU Compute Engine (batch-aware FIFO)" << std::endl
            << "     -> AI Inference Engine" << std::endl
            << "     -> Optimized Video Output" << std::endl
            << "     -> Resource Manager (centralized)" << std::endl
            << "     -> Runtime (lifecycle controller)" << std::endl
            << "     -> EventBus (module communication)";
        LIZ_INFO(oss.str());
    }

    std::cout << std::endl;

    // -- 15. Cleanup -----------------------------------------------------------
    gpu_compute.shutdown();
    pool.shutdown();
    ai_pipeline.clear_processors();
    gpu_ctx.shutdown();
    engine.shutdown();

    std::cout << std::endl;
    return 0;
}