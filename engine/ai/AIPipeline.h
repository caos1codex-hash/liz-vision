#pragma once

#include "engine/ai/AIContext.h"
#include "engine/ai/AIProcessor.h"
#include "engine/video/VideoFrame.h"

#include <cstddef>
#include <memory>
#include <vector>

namespace liz {

// Forward declarations.
class Engine;

/// Statistics from an AI pipeline run.
struct AIPipelineStats {
    std::size_t frames_input      = 0;
    std::size_t frames_output     = 0;
    std::size_t processors_used   = 0;
    std::size_t tasks_submitted   = 0;
    std::size_t tasks_completed   = 0;
    std::size_t tasks_failed      = 0;
    double      total_time_ms     = 0.0;
};

/// Orchestrates AI processing of video frames.
///
/// Flow:
///   1. Receive frames (from VideoPipeline or directly)
///   2. For each frame, for each registered AIProcessor:
///      a. Wrap the processing step into a Task
///      b. Submit the Task to the Engine's Scheduler
///   3. Execute all tasks
///   4. Return processed frames (simulated in this sprint)
///
/// Processors are chained sequentially per frame:
///   Frame → Processor A → Processor B → Output
class AIPipeline {
public:
    AIPipeline() = default;

    // ── Processor management ──────────────────────────────────────────────────
    /// Add a processor to the pipeline (called in order).
    void add_processor(std::shared_ptr<AIProcessor> processor);

    /// Remove all processors (calls on_cleanup on each).
    void clear_processors();

    /// Number of registered processors.
    std::size_t processor_count() const;

    // ── Context ───────────────────────────────────────────────────────────────
    AIContext&       context();
    const AIContext& context() const;

    // ── Execution ─────────────────────────────────────────────────────────────
    /// Process a vector of frames through all registered processors.
    /// Creates Tasks for each (frame, processor) pair and runs them
    /// via the Engine's Scheduler.
    ///
    /// Returns pipeline statistics.
    AIPipelineStats run(Engine& engine, const std::vector<VideoFrame>& frames);

    /// Get statistics from the last run.
    const AIPipelineStats& last_stats() const;

private:
    AIContext                          context_;
    std::vector<std::shared_ptr<AIProcessor>> processors_;
    AIPipelineStats                    stats_;
};

} // namespace liz