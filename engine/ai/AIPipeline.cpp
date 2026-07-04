#include "engine/ai/AIPipeline.h"
#include "engine/core/Engine.h"
#include "engine/core/Logger.h"

#include <chrono>
#include <sstream>

namespace liz {

// ── Processor management ─────────────────────────────────────────────────────
void AIPipeline::add_processor(std::shared_ptr<AIProcessor> processor) {
    if (!processor) {
        LIZ_WARN("AIPipeline: attempted to add null processor");
        return;
    }

    processor->on_init();

    std::ostringstream oss;
    oss << "AIPipeline: added processor '"
        << processor->name() << "' ["
        << ai_processor_type_to_string(processor->type()) << "]";
    LIZ_INFO(oss.str());

    processors_.push_back(std::move(processor));
}

void AIPipeline::clear_processors() {
    for (auto& proc : processors_) {
        proc->on_cleanup();
    }
    auto count = processors_.size();
    processors_.clear();

    std::ostringstream oss;
    oss << "AIPipeline: cleared " << count << " processors";
    LIZ_INFO(oss.str());
}

std::size_t AIPipeline::processor_count() const {
    return processors_.size();
}

// ── Context ──────────────────────────────────────────────────────────────────
AIContext&       AIPipeline::context()       { return context_; }
const AIContext& AIPipeline::context() const { return context_; }

// ── Execution ────────────────────────────────────────────────────────────────
AIPipelineStats AIPipeline::run(Engine& engine,
                                 const std::vector<VideoFrame>& frames) {
    // Reset stats
    stats_ = AIPipelineStats{};
    stats_.frames_input    = frames.size();
    stats_.processors_used = processors_.size();

    LIZ_INFO("========== AIPipeline START ==========");

    {
        std::ostringstream oss;
        oss << "AI Context: " << context_.summary();
        LIZ_INFO(oss.str());
    }

    if (processors_.empty()) {
        LIZ_WARN("AIPipeline: no processors registered — frames pass through unchanged");
        stats_.frames_output = frames.size();
        LIZ_INFO("========== AIPipeline END (passthrough) ==========");
        return stats_;
    }

    if (frames.empty()) {
        LIZ_WARN("AIPipeline: no frames to process");
        LIZ_INFO("========== AIPipeline END (empty) ==========");
        return stats_;
    }

    auto total_start = std::chrono::steady_clock::now();

    // Submit tasks: one task per (frame, processor) pair.
    // We store a snapshot of each frame's metadata in the lambda
    // so the simulated processing has something to log.
    for (const auto& frame : frames) {
        for (const auto& proc : processors_) {
            // Capture by value for the lambda.
            auto fid  = frame.frame_id();
            auto fw   = frame.width();
            auto fh   = frame.height();
            auto fts  = frame.timestamp_ms();
            auto fname = std::string(proc->name());
            auto ftype = ai_processor_type_to_string(proc->type());

            engine.submit_task(
                "AI " + fname + " on Frame #" + std::to_string(fid),
                [fid, fw, fh, fts, fname, ftype]() {
                    std::ostringstream oss;
                    oss << "AI processing step: [" << ftype << "] '"
                        << fname << "' on Frame #" << fid
                        << " [" << fw << "x" << fh << "]"
                        << " ts=" << fts << "ms"
                        << " (simulated inference)";
                    LIZ_INFO(oss.str());
                    return true;
                }
            );
            ++stats_.tasks_submitted;
        }
    }

    {
        std::ostringstream oss;
        oss << "AIPipeline: submitted " << stats_.tasks_submitted << " AI tasks ("
            << stats_.frames_input << " frames x "
            << stats_.processors_used << " processors)";
        LIZ_INFO(oss.str());
    }

    // Execute all tasks via the Engine's Scheduler.
    auto executed = engine.run_pending();
    stats_.tasks_completed = executed;
    stats_.tasks_failed    = stats_.tasks_submitted - executed;
    stats_.frames_output   = frames.size();  // simulated: all frames produce output

    auto total_end = std::chrono::steady_clock::now();
    stats_.total_time_ms =
        std::chrono::duration<double, std::milli>(total_end - total_start).count();

    // Summary
    {
        std::ostringstream oss;
        oss << "AIPipeline complete: "
            << stats_.frames_input << " frames in | "
            << stats_.frames_output << " frames out | "
            << stats_.tasks_completed << "/" << stats_.tasks_submitted << " tasks | "
            << stats_.tasks_failed << " failed | "
            << stats_.total_time_ms << " ms";
        LIZ_INFO(oss.str());
    }

    LIZ_INFO("========== AIPipeline END ==========");
    return stats_;
}

// ── Accessors ────────────────────────────────────────────────────────────────
const AIPipelineStats& AIPipeline::last_stats() const {
    return stats_;
}

} // namespace liz