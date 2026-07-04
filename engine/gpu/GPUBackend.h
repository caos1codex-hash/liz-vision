#pragma once

#include "engine/video/VideoFrame.h"

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace liz {

/// Memory handle returned by a GPU backend after allocation.
/// Opaque to the caller — backend implementations interpret it.
using GPUMemoryHandle = void*;

/// Abstract interface for GPU (or CPU-fallback) compute backends.
///
/// Every backend must implement this interface so the GPUContext
/// can route work uniformly regardless of the actual hardware.
///
/// In this sprint:
///   - CPUBackend is implemented (does work on CPU, simulates GPU API)
///   - CUDABackend is NOT implemented (placeholder for future sprints)
class GPUBackend {
public:
    virtual ~GPUBackend() = default;

    // ── Identity ──────────────────────────────────────────────────────────────
    virtual std::string_view name() const = 0;

    // ── Lifecycle ─────────────────────────────────────────────────────────────
    /// Initialize the backend.  Returns true on success.
    virtual bool initialize() = 0;

    /// Shut down the backend and release all resources.
    virtual void shutdown() = 0;

    /// True after a successful initialize().
    virtual bool is_initialized() const = 0;

    // ── Memory management ─────────────────────────────────────────────────────
    /// Allocate a memory buffer on the device.
    /// @param bytes  Number of bytes to allocate.
    /// @return       Handle to the allocated memory, or nullptr on failure.
    virtual GPUMemoryHandle allocate_memory(std::size_t bytes) = 0;

    /// Release a previously allocated memory buffer.
    virtual void release_memory(GPUMemoryHandle handle) = 0;

    // ── Frame transfer ────────────────────────────────────────────────────────
    /// Upload a VideoFrame's pixel data to the device.
    /// Returns a handle to the device-side copy.
    virtual GPUMemoryHandle upload_frame(const VideoFrame& frame) = 0;

    /// Download device data back into a VideoFrame.
    /// The output frame is created with the given dimensions and data.
    virtual VideoFrame download_frame(GPUMemoryHandle handle,
                                       std::uint32_t width,
                                       std::uint32_t height) = 0;

    // ── Compute ───────────────────────────────────────────────────────────────
    /// Execute a named kernel/operation on device memory.
    /// In this sprint: logs the operation name, simulates execution.
    /// @param kernel_name  Name of the operation (e.g. "upscale", "interpolate").
    /// @param input        Handle to input data.
    /// @param output_size  Expected output size in bytes.
    /// @return             Handle to the output data, or nullptr on failure.
    virtual GPUMemoryHandle execute_kernel(std::string_view kernel_name,
                                            GPUMemoryHandle input,
                                            std::size_t output_size) = 0;

    // ── Info ──────────────────────────────────────────────────────────────────
    /// Return a human-readable device info string (e.g. "CPU Fallback", "CUDA: RTX 4090").
    virtual std::string device_info() const = 0;

    /// Total device memory in bytes (0 if unknown / CPU).
    virtual std::size_t device_memory_total() const = 0;

    /// Free device memory in bytes (0 if unknown / CPU).
    virtual std::size_t device_memory_free() const = 0;
};

} // namespace liz