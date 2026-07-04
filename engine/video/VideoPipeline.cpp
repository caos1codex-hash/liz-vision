#include "engine/video/VideoPipeline.h"
#include "engine/core/Engine.h"
#include "engine/core/Logger.h"

#include <chrono>
#include <sstream>

namespace liz {

// ── Batch mode (Sprint 2 API — preserved) ─────────────────────────────────────
PipelineStats VideoPipeline::run(Engine& engine, const DecodeRequest& request) {
    stats_ = PipelineStats{};

    LIZ_INFO("========== VideoPipeline START (batch) ==========");

    LIZ_INFO("[Phase 1/3] Decoding video frames...");

    auto decode_start = std::chrono::steady_clock::now();

    VideoDecoder decoder;
    DecodeResult result = decoder.decode(request);

    auto decode_end = std::chrono::steady_clock::now();
    stats_.decode_time_ms =
        std::chrono::duration<double, std::milli>(decode_end - decode_start).count();

    if (!result.success) {
        LIZ_ERROR("VideoPipeline: decode failed — " + result.error_msg);
        LIZ_INFO("========== VideoPipeline ABORTED ==========");
        return stats_;
    }

    stats_.frames_received = result.frames.size();

    {
        std::ostringstream oss;
        oss << "[Phase 1/3] Decoded " << stats_.frames_received
            << " frames in " << stats_.decode_time_ms << " ms";
        LIZ_INFO(oss.str());
    }

    LIZ_INFO("[Phase 2/3] Submitting frame tasks to scheduler...");

    for (const auto& frame : result.frames) {
        auto fid = frame.frame_id();
        auto fw  = frame.width();
        auto fh  = frame.height();
        auto fts = frame.timestamp_ms();
        auto fdata_size = frame.data_size_bytes();

        engine.submit_task(
            "Process Frame #" + std::to_string(fid),
            [fid, fw, fh, fts, fdata_size]() {
                std::ostringstream oss;
                oss << "Processing Frame #" << fid
                    << " [" << fw << "x" << fh << "]"
                    << " ts=" << fts << "ms"
                    << " size=" << fdata_size << " bytes"
                    << " (simulated pass-through)";
                LIZ_INFO(oss.str());
                return true;
            }
        );
        ++stats_.tasks_submitted;
    }

    {
        std::ostringstream oss;
        oss << "[Phase 2/3] Submitted " << stats_.tasks_submitted << " frame tasks";
        LIZ_INFO(oss.str());
    }

    LIZ_INFO("[Phase 3/3] Executing pipeline tasks...");

    auto exec_start = std::chrono::steady_clock::now();
    auto executed = engine.run_pending();
    auto exec_end = std::chrono::steady_clock::now();
    stats_.execution_time_ms =
        std::chrono::duration<double, std::milli>(exec_end - exec_start).count();

    stats_.tasks_completed = executed;
    stats_.tasks_failed = stats_.tasks_submitted - executed;

    {
        std::ostringstream oss;
        oss << "[Phase 3/3] Executed " << executed << " tasks in "
            << stats_.execution_time_ms << " ms";
        LIZ_INFO(oss.str());
    }

    {
        std::ostringstream oss;
        oss << "Pipeline complete: "
            << stats_.frames_received << " frames | "
            << stats_.tasks_submitted << " tasks | "
            << stats_.tasks_completed << " completed | "
            << stats_.tasks_failed << " failed | "
            << "decode=" << stats_.decode_time_ms << "ms | "
            << "exec=" << stats_.execution_time_ms << "ms";
        LIZ_INFO(oss.str());
    }

    LIZ_INFO("========== VideoPipeline END (batch) ==========");
    return stats_;
}

// ── Streaming mode (Sprint 4 — frame-by-frame with GPU routing) ───────────────
PipelineStats VideoPipeline::run_streaming(
        Engine& engine,
        const std::string& file_path,
        GPUContext& gpu_ctx,
        std::function<void(const VideoFrame&, GPUContext&)> process_fn)
{
    stats_ = PipelineStats{};
    stats_.streaming_mode = true;
    stats_.gpu_routed = gpu_ctx.is_initialized();

    LIZ_INFO("========== VideoPipeline START (streaming) ==========");

    // ── Open file via FFmpegDecoder ───────────────────────────────────────────
    FFmpegDecoder ffmpeg;
    auto open_result = ffmpeg.open(file_path);

    if (!open_result.success) {
        LIZ_ERROR("VideoPipeline: failed to open '" + file_path + "' — " + open_result.error_msg);
        LIZ_INFO("========== VideoPipeline ABORTED ==========");
        return stats_;
    }

    if (open_result.using_stub) {
        LIZ_INFO("VideoPipeline: using stub decoder (FFmpeg not linked)");
    }

    {
        std::ostringstream oss;
        oss << "VideoPipeline: opened '" << file_path << "' — "
            << open_result.width << "x" << open_result.height
            << " @ " << open_result.fps << " fps, "
            << open_result.frame_count << " frames";
        LIZ_INFO(oss.str());
    }

    // ── Stream frames one-by-one ──────────────────────────────────────────────
    auto total_start = std::chrono::steady_clock::now();
    std::uint32_t frame_idx = 0;

    while (auto frame = ffmpeg.decode_next_frame()) {
        // Extract all values BEFORE moving the frame into the lambda.
        std::uint32_t fid   = frame->frame_id();
        std::uint32_t fw    = frame->width();
        std::uint32_t fh    = frame->height();
        double        fts   = frame->timestamp_ms();
        std::size_t   fsize = frame->data_size_bytes();

        // Upload to GPU context (routing decision logged inside).
        auto gpu_handle = gpu_ctx.upload_frame(*frame);

        ++stats_.frames_received;

        // Submit task for this frame.
        engine.submit_task(
            "Stream Frame #" + std::to_string(fid),
            [fid, fw, fh, fts, fsize, gpu_handle]() {
                std::ostringstream oss;
                oss << "Streaming Frame #" << fid
                    << " [" << fw << "x" << fh << "]"
                    << " ts=" << fts << "ms"
                    << " size=" << fsize << " bytes"
                    << " GPU=" << (gpu_handle ? "uploaded" : "skipped");
                LIZ_INFO(oss.str());
                return true;
            }
        );

        // Release GPU memory for this frame after task submission.
        // (In streaming mode the task doesn't do GPU work — GPU routing
        //  is logged during upload, and memory is released here.)
        if (gpu_handle) {
            gpu_ctx.release_memory(gpu_handle);
        }
        ++stats_.tasks_submitted;

        // Execute immediately (true streaming: 1 frame → process → next).
        engine.run_next();
        ++frame_idx;
    }

    auto total_end = std::chrono::steady_clock::now();
    stats_.execution_time_ms =
        std::chrono::duration<double, std::milli>(total_end - total_start).count();
    stats_.tasks_completed = stats_.tasks_submitted;  // streaming: immediate execution

    // ── Summary ───────────────────────────────────────────────────────────────
    {
        std::ostringstream oss;
        oss << "Streaming pipeline complete: "
            << stats_.frames_received << " frames streamed | "
            << stats_.tasks_submitted << " tasks | "
            << stats_.tasks_completed << " completed | "
            << "gpu=" << (stats_.gpu_routed ? "routed" : "bypass") << " | "
            << "time=" << stats_.execution_time_ms << "ms";
        LIZ_INFO(oss.str());
    }

    ffmpeg.close();
    LIZ_INFO("========== VideoPipeline END (streaming) ==========");
    return stats_;
}

// ── Accessors ─────────────────────────────────────────────────────────────────
const PipelineStats& VideoPipeline::last_stats() const {
    return stats_;
}

} // namespace liz