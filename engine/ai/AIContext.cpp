#include "engine/ai/AIContext.h"

#include <sstream>

namespace liz {

// ── Enum converters ───────────────────────────────────────────────────────────
const char* device_type_to_string(DeviceType type) {
    switch (type) {
        case DeviceType::CPU:    return "CPU";
        case DeviceType::CUDA:   return "CUDA";
        case DeviceType::ROCm:   return "ROCm";
        case DeviceType::Vulkan: return "Vulkan";
    }
    return "Unknown";
}

const char* precision_mode_to_string(PrecisionMode mode) {
    switch (mode) {
        case PrecisionMode::FP32: return "FP32";
        case PrecisionMode::FP16: return "FP16";
        case PrecisionMode::INT8: return "INT8";
    }
    return "Unknown";
}

// ── Device ────────────────────────────────────────────────────────────────────
DeviceType AIContext::device_type() const { return device_type_; }
void       AIContext::set_device_type(DeviceType type) { device_type_ = type; }

// ── Precision ─────────────────────────────────────────────────────────────────
PrecisionMode AIContext::precision_mode() const { return precision_; }
void         AIContext::set_precision_mode(PrecisionMode mode) { precision_ = mode; }

// ── Model ─────────────────────────────────────────────────────────────────────
const std::string& AIContext::model_name() const { return model_name_; }
void               AIContext::set_model_name(std::string name) { model_name_ = std::move(name); }

// ── Batch ─────────────────────────────────────────────────────────────────────
std::uint32_t AIContext::batch_size() const { return batch_size_; }
void          AIContext::set_batch_size(std::uint32_t size) { batch_size_ = size; }

// ── Defaults & summary ────────────────────────────────────────────────────────
void AIContext::load_defaults() {
    device_type_ = DeviceType::CPU;
    precision_   = PrecisionMode::FP32;
    model_name_  = "none";
    batch_size_  = 1;
}

std::string AIContext::summary() const {
    std::ostringstream oss;
    oss << "device=" << device_type_to_string(device_type_)
        << " precision=" << precision_mode_to_string(precision_)
        << " model='" << model_name_ << "'"
        << " batch=" << batch_size_;
    return oss.str();
}

} // namespace liz