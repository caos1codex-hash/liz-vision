#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

namespace liz {

/// Point-in-time snapshot of engine performance metrics.
///
/// Captured by DiagnosticsManager::capture_snapshot().
/// All values are simulated or sourced from existing subsystems.
struct PerformanceSnapshot {
    // -- Timing --
    double fps              = 0.0;
    double frame_time_ms    = 0.0;
    double cpu_time_ms      = 0.0;
    double gpu_time_ms      = 0.0;

    // -- Memory (simulated / from GPUMemoryPool) --
    std::uint64_t ram_used_bytes   = 0;
    std::uint64_t vram_used_bytes  = 0;

    // -- Counters --
    std::size_t events_published   = 0;
    std::size_t services_active    = 0;
    std::size_t resources_active   = 0;
    std::size_t tasks_pending      = 0;

    // -- Metadata --
    std::uint64_t timestamp_ms     = 0;
    std::string   label;

    /// Generate a formatted summary string.
    std::string to_string() const;
};

} // namespace liz
