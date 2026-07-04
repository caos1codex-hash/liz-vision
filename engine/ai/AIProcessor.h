#pragma once

#include "engine/video/VideoFrame.h"

#include <string>
#include <string_view>

namespace liz {

/// Supported AI processor types (for future real implementations).
enum class AIProcessorType {
    Upscaler,       ///< Frame super-resolution (e.g. 1080p → 4K)
    Interpolator,   ///< Frame interpolation (e.g. 24fps → 120fps)
    Denoiser,       ///< Noise reduction
    Enhancer,       ///< General quality enhancement
    Custom          ///< User-defined processor
};

/// Convert an AIProcessorType to a human-readable string.
const char* ai_processor_type_to_string(AIProcessorType type);

/// Abstract base class for all AI frame processors.
///
/// Each processor takes a VideoFrame as input and produces a
/// processed VideoFrame as output.  In this sprint the processing
/// is simulated — no real AI runs.
///
/// Concrete processors (Upscaler, Interpolator, etc.) will be
/// implemented in future sprints with actual model inference.
class AIProcessor {
public:
    virtual ~AIProcessor() = default;

    // ── Identity ──────────────────────────────────────────────────────────────
    /// Human-readable name (e.g. "RealESRGAN Upscaler").
    virtual std::string_view name() const = 0;

    /// The category this processor belongs to.
    virtual AIProcessorType type() const = 0;

    // ── Processing ────────────────────────────────────────────────────────────
    /// Process a single frame.  Returns the processed frame.
    /// In this sprint: returns a copy with metadata marking it as "processed".
    virtual VideoFrame process_frame(const VideoFrame& input) = 0;

    // ── Lifecycle ─────────────────────────────────────────────────────────────
    /// Called when the processor is added to a pipeline.
    virtual void on_init() = 0;

    /// Called when the processor is removed from a pipeline.
    virtual void on_cleanup() = 0;
};

} // namespace liz