#pragma once

#include "engine/ai/tensor/TensorShape.h"

#include <cstdint>
#include <string>
#include <vector>

namespace liz {

/// Semantic purpose of a tensor in the pipeline.
enum class TensorType {
    Frame,        ///< Raw decoded video frame data
    FeatureMap,   ///< Intermediate CNN feature map
    ModelInput,   ///< Preprocessed input ready for inference
    ModelOutput   ///< Inference result tensor
};

/// Convert TensorType to a readable string.
const char* tensor_type_to_string(TensorType type);

/// A multi-dimensional tensor backed by a flat float32 buffer.
///
/// Pure data structure — no GPU, no ML, no real computation.
/// Used as the interchange format between pipeline stages.
class Tensor {
public:
    /// Default-construct an empty tensor.
    Tensor() = default;

    /// Construct with shape and type.  Data is zero-initialized.
    Tensor(TensorShape shape, TensorType type = TensorType::Frame);

    /// Construct from raw uint8 data (e.g. VideoFrame pixels).
    /// Converts each byte to float32 [0.0, 255.0].
    Tensor(TensorShape shape, TensorType type,
           const std::vector<std::uint8_t>& raw_bytes);

    // -- Shape ------------------------------------------------------------------
    const TensorShape& shape() const;

    /// Change dimensions.  Data is preserved if new size <= old size,
    /// otherwise zero-padded.
    void reshape(const TensorShape& new_shape);

    // -- Type -------------------------------------------------------------------
    TensorType type() const;
    void       set_type(TensorType t);

    // -- Data -------------------------------------------------------------------
    std::size_t size() const;  // number of float elements

    /// Direct mutable access to the float buffer.
    std::vector<float>&       data();
    const std::vector<float>& data() const;

    /// Total bytes occupied by the float buffer.
    std::size_t byte_size() const;

    // -- Info -------------------------------------------------------------------
    /// One-line summary: "Tensor{type=WxHxC elements=N bytes=B}".
    std::string info() const;

    // -- Static helpers ---------------------------------------------------------
    /// Create a tensor from a raw uint8 pixel buffer (RGB24).
    static Tensor from_frame_data(std::uint32_t width,
                                   std::uint32_t height,
                                   const std::vector<std::uint8_t>& pixels);

private:
    TensorShape           shape_;
    TensorType            type_  = TensorType::Frame;
    std::vector<float>    data_;
};

} // namespace liz