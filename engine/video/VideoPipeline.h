#pragma once

#include "engine/video/VideoDecoder.h"
#include "engine/video/ffmpeg/FFmpegDecoder.h"
#include "engine/gpu/GPUContext.h"

#include <cstddef>
#include <functional>
#include <string>
#include <vector>

namespace liz {

// Forward declaration — avoids pulling Engine.h into the pipeline header.
class Engine;

/// Statistics from a pipeline run.
struct PipelineStats {
    std::size_t frames_received   = 0;
    std::size_t tasks_submitted   = 0;
    std::size_t tasks_completed   = 0;
    std::size_t tasks_failed      = 0;
    double      decode_time_ms    = 0.0;
    double      execution_time_ms = 0.0;
    bool        streaming_mode    = false;
    bool        gpu_routed        = false;
};

/// Orchestrates the video processing pipeline.
///
/// Supports two modes:
///   1. Batch mode (Sprint 2): decode all frames, then process.
///   2. Streaming mode (Sprint 4): decode one frame, process it, repeat.
///
/// Flow (streaming):
///   FFmpegDecoder::decode_next_frame()
///     → GPUContext::upload_frame()
///       → Task submitted to Scheduler
///         → Task executed (GPU kernel simulated)
///           → Output
class VideoPipeline {
public:
    VideoPipeline() = default;

    /// Batch mode: decode all at once, then create tasks and execute.
    /// (Original Sprint 2 API — preserved for compatibility.)
    PipelineStats run(Engine& engine, const DecodeRequest& request);

    /// Streaming mode: decode frames one-by-one via FFmpegDecoder,
    /// uploading each to GPU context, creating a task per frame.
    /// @param engine       Reference to the initialized Engine.
    /// @param file_path    Path to the video file (real or simulated).
    /// @param gpu_ctx      GPU context for frame routing.
    /// @param process_fn   Optional per-frame processing callback.
    PipelineStats run_streaming(Engine& engine,
                                 const std::string& file_path,
                                 GPUContext& gpu_ctx,
                                 std::function<void(const VideoFrame&, GPUContext&)> process_fn = {});

    /// Get statistics from the last run.
    const PipelineStats& last_stats() const;

private:
    PipelineStats stats_;
};

} // namespace liz