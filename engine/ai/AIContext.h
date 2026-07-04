#pragma once

#include <cstdint>
#include <string>

namespace liz {

/// Supported compute devices.
enum class DeviceType {
    CPU,    ///< CPU inference (only mode in this sprint)
    CUDA,   ///< NVIDIA GPU (future)
    ROCm,   ///< AMD GPU (future)
    Vulkan  ///< Cross-platform GPU (future)
};

/// Convert a DeviceType to a string.
const char* device_type_to_string(DeviceType type);

/// Supported precision modes for inference.
enum class PrecisionMode {
    FP32,   ///< 32-bit floating point
    FP16,   ///< 16-bit floating point (future)
    INT8    ///< 8-bit integer quantized (future)
};

/// Convert a PrecisionMode to a string.
const char* precision_mode_to_string(PrecisionMode mode);

/// Holds AI runtime configuration — device, precision, model info.
///
/// In this sprint everything is placeholder.  Future sprints will
/// populate these fields from Config and use them for real inference.
class AIContext {
public:
    AIContext() = default;

    // ── Device ────────────────────────────────────────────────────────────────
    DeviceType       device_type() const;
    void             set_device_type(DeviceType type);

    // ── Precision ─────────────────────────────────────────────────────────────
    PrecisionMode    precision_mode() const;
    void             set_precision_mode(PrecisionMode mode);

    // ── Model ─────────────────────────────────────────────────────────────────
    const std::string& model_name() const;
    void               set_model_name(std::string name);

    // ── Batch ─────────────────────────────────────────────────────────────────
    std::uint32_t batch_size() const;
    void          set_batch_size(std::uint32_t size);

    // ── Convenience ───────────────────────────────────────────────────────────
    /// Load sensible defaults for this sprint (CPU, FP32, no model).
    void load_defaults();

    /// Produce a one-line summary string for logging.
    std::string summary() const;

private:
    DeviceType    device_type_   = DeviceType::CPU;
    PrecisionMode precision_     = PrecisionMode::FP32;
    std::string   model_name_;
    std::uint32_t batch_size_    = 1;
};

} // namespace liz