#pragma once

#include "engine/diagnostics/Profiler.h"
#include "engine/diagnostics/PerformanceSnapshot.h"
#include "engine/diagnostics/EngineStatistics.h"

#include <cstddef>
#include <string>
#include <vector>

namespace liz {

/// Data provider interface for the DiagnosticsManager.
///
/// Instead of reaching into subsystems directly (which would create
/// circular dependencies), the DiagnosticsManager accepts data through
/// this provider.  The demo or a future integration layer implements
/// this interface to feed real metrics.
///
/// This sprint: DiagnosticsDataProvider is a simple struct with setters.
/// Future sprints: may become an abstract interface.
class DiagnosticsDataProvider {
public:
    DiagnosticsDataProvider() = default;

    // -- Performance metrics (sourced from PerformanceManager, etc.) --
    double   fps               = 0.0;
    double   frame_time_ms     = 0.0;
    double   cpu_time_ms       = 0.0;
    double   gpu_time_ms       = 0.0;
    std::uint64_t ram_used     = 0;
    std::uint64_t vram_used    = 0;

    // -- Counters (sourced from existing subsystems) --
    std::size_t events_published    = 0;
    std::size_t services_active     = 0;
    std::size_t resources_active    = 0;
    std::size_t tasks_pending       = 0;
    std::size_t frames_processed    = 0;
    std::size_t services_registered = 0;
    std::size_t resources_loaded    = 0;
    std::size_t plugins_active      = 0;
    std::size_t batches_executed    = 0;
    std::size_t inferences_executed = 0;
    std::size_t gpu_commands        = 0;
    double     total_runtime_ms     = 0.0;

    // -- Assets (Sprint 14) --
    std::size_t assets_loaded        = 0;
    std::size_t assets_active        = 0;
};

/// Central diagnostics and profiling manager for the LIZ Vision engine.
///
/// Provides:
///   - Profiling sessions (via Profiler)
///   - Point-in-time performance snapshots
///   - Cumulative engine statistics
///   - Formatted diagnostic reports
///
/// Does NOT own any engine subsystem — receives data through
/// DiagnosticsDataProvider or direct setter methods.
///
/// Usage:
///   DiagnosticsManager diag;
///   diag.set_provider(provider);
///   diag.capture_snapshot("after_init");
///   diag.print_report();
class DiagnosticsManager {
public:
    DiagnosticsManager();
    ~DiagnosticsManager() = default;

    // Non-copyable.
    DiagnosticsManager(const DiagnosticsManager&) = delete;
    DiagnosticsManager& operator=(const DiagnosticsManager&) = delete;

    // ── Data source ────────────────────────────────────────────────────────

    /// Set the data provider.  Does NOT take ownership.
    void set_provider(const DiagnosticsDataProvider* provider);

    // ── Profiler access ──────────────────────────────────────────────────

    /// Get the internal profiler for start/stop/section operations.
    Profiler& profiler();
    const Profiler& profiler() const;

    // ── Snapshots ─────────────────────────────────────────────────────────

    /// Capture a point-in-time snapshot using the current provider data.
    /// If no provider is set, captures zeros.
    PerformanceSnapshot capture_snapshot(const std::string& label);

    /// Get all captured snapshots.
    const std::vector<PerformanceSnapshot>& snapshots() const;

    /// Number of snapshots captured so far.
    std::size_t snapshot_count() const;

    // ── Statistics ────────────────────────────────────────────────────────

    /// Gather cumulative statistics from the current provider data.
    EngineStatistics statistics() const;

    // ── Reporting ─────────────────────────────────────────────────────────

    /// Print a full diagnostic report to the Logger.
    void print_report() const;

    // ── Reset ────────────────────────────────────────────────────────────

    /// Reset all diagnostics: profiler, snapshots, statistics.
    void reset();

private:
    Profiler profiler_;
    const DiagnosticsDataProvider* provider_ = nullptr;
    std::vector<PerformanceSnapshot> snapshots_;
};

} // namespace liz
