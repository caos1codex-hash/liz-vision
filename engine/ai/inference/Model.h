#pragma once

#include "engine/video/VideoFrame.h"

#include <string>
#include <string_view>

namespace liz {

/// Supported model types for AI enhancement.
enum class ModelType {
    Upscaler,       ///< Frame super-resolution (e.g. 1080p -> 4K)
    Interpolator,   ///< Frame interpolation (e.g. 24fps -> 120fps)
    Denoiser        ///< Noise reduction
};

/// Convert a ModelType to a human-readable string.
const char* model_type_to_string(ModelType type);

/// Describes the tensor format a model expects or produces.
struct TensorFormat {
    std::string name        = "RGB24";   ///< e.g. "RGB24", "NV12", "FP32_NCHW"
    std::uint32_t channels  = 3;
    std::string dtype      = "uint8";   ///< e.g. "uint8", "float32"
};

/// Abstract base class for all AI models in LIZ Vision.
///
/// Each model has an identity (name, version, type), declares its
/// input and output tensor formats, and exposes a single run() method
/// that processes a VideoFrame and returns an enhanced VideoFrame.
///
/// In this sprint: run() is simulated — no real ML inference.
/// Future sprints will subclass this with ONNX / TensorRT backends.
class Model {
public:
    virtual ~Model() = default;

    // -- Identity -----------------------------------------------------------------
    /// Human-readable model name (e.g. "liz_upscaler_v1").
    virtual std::string_view name() const = 0;

    /// Model version string (e.g. "1.0.0").
    virtual std::string_view version() const = 0;

    /// The category this model belongs to.
    virtual ModelType type() const = 0;

    // -- Format -------------------------------------------------------------------
    /// The input tensor format the model expects.
    virtual TensorFormat input_format() const = 0;

    /// The output tensor format the model produces.
    virtual TensorFormat output_format() const = 0;

    // -- Inference ----------------------------------------------------------------
    /// Run the model on a single input frame.
    /// Returns the processed (enhanced) frame.
    ///
    /// In this sprint: returns a simulated copy with metadata.
    /// Future: actual ONNX / TensorRT inference.
    virtual VideoFrame run(const VideoFrame& input_frame) = 0;

    // -- Lifecycle ----------------------------------------------------------------
    /// Called when the model is loaded into the InferenceEngine.
    virtual void on_load() = 0;

    /// Called when the model is unloaded.
    virtual void on_unload() = 0;

    // -- Utility ------------------------------------------------------------------
    /// Produce a one-line summary for logging.
    std::string summary() const;
};

} // namespace liz