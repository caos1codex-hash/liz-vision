#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

namespace liz {

/// Describes the spatial and channel dimensions of a tensor.
/// Layout: {width, height, channels}.
struct TensorShape {
    std::uint32_t width    = 0;
    std::uint32_t height   = 0;
    std::uint32_t channels = 0;

    /// Total element count: width * height * channels.
    std::size_t element_count() const;

    /// Byte size for float32 storage.
    std::size_t byte_size() const;

    /// Human-readable: "WxHxC"
    std::string to_string() const;
};

} // namespace liz