#include "engine/gpu/GPUDevice.h"
#include "engine/core/Logger.h"

#include <sstream>

namespace liz {

// ── Probe ─────────────────────────────────────────────────────────────────────
void GPUDevice::probe() {
    devices_.clear();

    // Always add CPU fallback.
    GPUDeviceInfo cpu;
    cpu.name      = "CPU Fallback";
    cpu.vendor    = "System CPU";
    cpu.api       = "CPU";
    cpu.is_gpu    = false;
    cpu.memory_total_mb = 0;
    devices_.push_back(cpu);

    // Future sprints:
    // - Try CUDA (cuDeviceGetCount, cuDeviceGet)
    // - Try ROCm (hipGetDeviceCount)
    // - Try Vulkan (vkEnumeratePhysicalDevices)

    std::ostringstream oss;
    oss << "GPUDevice: probed " << devices_.size() << " device(s)";
    LIZ_INFO(oss.str());
}

// ── Accessors ──────────────────────────────────────────────────────────────────
std::size_t GPUDevice::device_count() const {
    return devices_.size();
}

const GPUDeviceInfo& GPUDevice::device(std::size_t index) const {
    return devices_.at(index);
}

bool GPUDevice::has_gpu() const {
    for (const auto& d : devices_) {
        if (d.is_gpu) return true;
    }
    return false;
}

const GPUDeviceInfo& GPUDevice::best_device() const {
    for (const auto& d : devices_) {
        if (d.is_gpu) return d;
    }
    return devices_.front();
}

// ── Summary ────────────────────────────────────────────────────────────────────
std::string GPUDevice::summary() const {
    std::ostringstream oss;
    for (std::size_t i = 0; i < devices_.size(); ++i) {
        const auto& d = devices_[i];
        if (i > 0) oss << " | ";
        oss << "[" << i << "] " << d.name
            << " (" << d.api << ", " << d.vendor << ")";
        if (d.memory_total_mb > 0) {
            oss << " " << d.memory_total_mb << "MB";
        }
    }
    return oss.str();
}

} // namespace liz