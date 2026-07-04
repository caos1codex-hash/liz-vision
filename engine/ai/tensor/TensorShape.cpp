#include "engine/ai/tensor/TensorShape.h"

#include <sstream>

namespace liz {

std::size_t TensorShape::element_count() const {
    return static_cast<std::size_t>(width)
         * static_cast<std::size_t>(height)
         * static_cast<std::size_t>(channels);
}

std::size_t TensorShape::byte_size() const {
    return element_count() * sizeof(float);
}

std::string TensorShape::to_string() const {
    std::ostringstream oss;
    oss << width << "x" << height << "x" << channels;
    return oss.str();
}

} // namespace liz