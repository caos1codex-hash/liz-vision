#pragma once

#include <cstddef>
#include <cstdint>

namespace liz {

/// Simple GPU memory buffer representation.
/// Only stores id and size — no real GPU memory is touched.
struct GPUBuffer {
    std::uint64_t id   = 0;
    std::size_t   size = 0;
};

} // namespace liz