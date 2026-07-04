#include "engine/ai/tensor/Tensor.h"

#include <algorithm>
#include <sstream>

namespace liz {

// -- Enum converter -----------------------------------------------------------
const char* tensor_type_to_string(TensorType type) {
    switch (type) {
        case TensorType::Frame:       return "Frame";
        case TensorType::FeatureMap:  return "FeatureMap";
        case TensorType::ModelInput:  return "ModelInput";
        case TensorType::ModelOutput: return "ModelOutput";
    }
    return "Unknown";
}

// -- Constructor / helpers ----------------------------------------------------

Tensor::Tensor(TensorShape shape, TensorType type)
    : shape_(shape), type_(type) {
    data_.resize(shape_.element_count(), 0.0f);
}

Tensor::Tensor(TensorShape shape, TensorType type,
               const std::vector<std::uint8_t>& raw_bytes)
    : shape_(shape), type_(type) {
    data_.reserve(raw_bytes.size());
    for (auto b : raw_bytes) {
        data_.push_back(static_cast<float>(b));
    }
}

Tensor Tensor::from_frame_data(std::uint32_t width,
                                std::uint32_t height,
                                const std::vector<std::uint8_t>& pixels) {
    TensorShape shape{width, height, 3};  // RGB24
    return Tensor(shape, TensorType::Frame, pixels);
}

// -- Shape --------------------------------------------------------------------

const TensorShape& Tensor::shape() const { return shape_; }

void Tensor::reshape(const TensorShape& new_shape) {
    auto new_count = new_shape.element_count();
    data_.resize(new_count, 0.0f);
    shape_ = new_shape;
}

// -- Type ---------------------------------------------------------------------

TensorType Tensor::type() const { return type_; }
void       Tensor::set_type(TensorType t) { type_ = t; }

// -- Data ---------------------------------------------------------------------

std::size_t Tensor::size() const { return data_.size(); }

std::vector<float>&       Tensor::data()       { return data_; }
const std::vector<float>& Tensor::data() const { return data_; }

std::size_t Tensor::byte_size() const { return data_.size() * sizeof(float); }

// -- Info ---------------------------------------------------------------------

std::string Tensor::info() const {
    std::ostringstream oss;
    oss << "Tensor{type=" << tensor_type_to_string(type_)
        << " shape=" << shape_.to_string()
        << " elements=" << data_.size()
        << " bytes=" << byte_size() << "}";
    return oss.str();
}

} // namespace liz