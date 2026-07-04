#pragma once

#include "engine/video/VideoDecoder.h"

#include <cstddef>
#include <string>
#include <vector>

namespace liz {

// Forward declaration — avoids pulling Engine.h into the pipeline header.
class Engine;

/// Statistics from a pipeline run.
struct PipelineStats {
    std::size_t frames_received  = 0;
    std::size_t tasks_submitted  = 0;
    std::size_t tasks_completed  = 0;
    std::size_t tasks_failed     = 0;
    double      decode_time_ms   = 0.0;
    double      execution_time_ms = 0.0;
};

/// Orchestrates the video processing pipeline.
///
/// Flow:
///   1. VideoDecoder generates fake frames
///   2. VideoPipeline wraps each frame into a Task
///   3. Tasks are submitted to the Engine's Scheduler
///   4. Tasks are executed sequentially (FIFO)
///
/// No real AI or video processing — each frame task simply logs
/// and simulates a pass-through.
class VideoPipeline {
public:
    VideoPipeline() = default;

    /// Run the full pipeline: decode → create tasks → execute.
    ///
    /// @param engine    Reference to the initialized Engine.
    /// @param request   Decode parameters (source, resolution, frame count).
    /// @return          Pipeline statistics.
    PipelineStats run(Engine& engine, const DecodeRequest& request);

    /// Get statistics from the last run.
    const PipelineStats& last_stats() const;

private:
    PipelineStats stats_;
};

} // namespace liz