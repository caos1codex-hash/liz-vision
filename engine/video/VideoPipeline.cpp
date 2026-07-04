#include "engine/video/VideoPipeline.h"
#include "engine/core/Engine.h"
#include "engine/core/Logger.h"

#include <chrono>
#include <sstream>

namespace liz {

// ── Public API ────────────────────────────────────────────────────────────────
PipelineStats VideoPipeline::run(Engine& engine, const DecodeRequest& request) {
    // Reset stats
    stats_ = PipelineStats{};

    LIZ_INFO("========== VideoPipeline START ==========");

    // ── Phase 1: Decode ───────────────────────────────────────────────────────
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

    // ── Phase 2: Submit tasks ─────────────────────────────────────────────────
    LIZ_INFO("[Phase 2/3] Submitting frame tasks to scheduler...");

    for (const auto& frame : result.frames) {
        // Capture frame info by value so the lambda owns its data.
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

    // ── Phase 3: Execute ──────────────────────────────────────────────────────
    LIZ_INFO("[Phase 3/3] Executing pipeline tasks...");

    auto exec_start = std::chrono::steady_clock::now();

    auto executed = engine.run_pending();

    auto exec_end = std::chrono::steady_clock::now();
    stats_.execution_time_ms =
        std::chrono::duration<double, std::milli>(exec_end - exec_start).count();

    stats_.tasks_completed = executed;
    // tasks_failed = submitted - completed (for simulated pass-through all succeed)
    stats_.tasks_failed = stats_.tasks_submitted - executed;

    {
        std::ostringstream oss;
        oss << "[Phase 3/3] Executed " << executed << " tasks in "
            << stats_.execution_time_ms << " ms";
        LIZ_INFO(oss.str());
    }

    // ── Summary ───────────────────────────────────────────────────────────────
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

    LIZ_INFO("========== VideoPipeline END ==========");
    return stats_;
}

// ── Accessors ─────────────────────────────────────────────────────────────────
const PipelineStats& VideoPipeline::last_stats() const {
    return stats_;
}

} // namespace liz