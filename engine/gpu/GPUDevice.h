#pragma once

#include <string>
#include <vector>

namespace liz {

/// Describes a compute device (GPU or CPU).
struct GPUDeviceInfo {
    std::string name;            ///< Device name (e.g. "CPU Fallback", "NVIDIA RTX 4090")
    std::string vendor;          ///< Vendor (e.g. "CPU", "NVIDIA", "AMD")
    std::string api;             ///< Compute API (e.g. "CPU", "CUDA", "ROCm", "Vulkan")
    std::size_t memory_total_mb  = 0;  ///< Total memory in MB (0 if unknown)
    int         compute_capability_major = 0;  ///< For CUDA: SM major (0 if N/A)
    int         compute_capability_minor = 0;  ///< For CUDA: SM minor (0 if N/A)
    bool        is_gpu = false;  ///< True if this is an actual GPU device
};

/// Queries available compute devices on the system.
///
/// In this sprint: always returns a single CPU device.
/// Future sprints: probe CUDA, ROCm, Vulkan.
class GPUDevice {
public:
    /// Probe the system for available compute devices.
    /// Populates the internal device list.
    void probe();

    /// Number of detected devices (always >= 1, the CPU fallback).
    std::size_t device_count() const;

    /// Get device info by index.
    const GPUDeviceInfo& device(std::size_t index) const;

    /// Check if any real GPU was detected.
    bool has_gpu() const;

    /// Get info for the best available device.
    /// Prefers GPU over CPU.
    const GPUDeviceInfo& best_device() const;

    /// Produce a summary string of all devices.
    std::string summary() const;

private:
    std::vector<GPUDeviceInfo> devices_;
};

} // namespace liz