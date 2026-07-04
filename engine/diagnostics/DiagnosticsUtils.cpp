#include "engine/diagnostics/PerformanceSnapshot.h"
#include "engine/diagnostics/EngineStatistics.h"

#include <sstream>
#include <iomanip>

namespace liz {

// ── PerformanceSnapshot ───────────────────────────────────────────────────

std::string PerformanceSnapshot::to_string() const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1);
    oss << "Snapshot[" << label << "]"
        << " fps=" << fps
        << " frame_time=" << frame_time_ms << "ms"
        << " cpu=" << cpu_time_ms << "ms"
        << " gpu=" << gpu_time_ms << "ms"
        << " ram=" << (ram_used_bytes / 1024) << "KB"
        << " vram=" << (vram_used_bytes / (1024 * 1024)) << "MB"
        << " events=" << events_published
        << " services=" << services_active
        << " resources=" << resources_active
        << " tasks=" << tasks_pending;
    return oss.str();
}

// ── EngineStatistics ──────────────────────────────────────────────────────

std::string EngineStatistics::to_string() const {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1);
    oss << "Statistics"
        << " runtime=" << total_runtime_ms << "ms"
        << " frames=" << frames_processed
        << " events=" << events_published
        << " services=" << services_registered
        << " resources=" << resources_loaded
        << " plugins=" << plugins_active
        << " batches=" << batches_executed
        << " inferences=" << inferences_executed
        << " gpu_commands=" << gpu_commands;
    return oss.str();
}

} // namespace liz
