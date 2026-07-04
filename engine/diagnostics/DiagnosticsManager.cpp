#include "engine/diagnostics/DiagnosticsManager.h"
#include "engine/diagnostics/PerformanceSnapshot.h"
#include "engine/diagnostics/EngineStatistics.h"
#include "engine/core/Logger.h"

#include <chrono>
#include <iostream>
#include <iomanip>
#include <sstream>

namespace liz {

// ── Constructor ───────────────────────────────────────────────────────────

DiagnosticsManager::DiagnosticsManager() = default;

// ── Data source ─────────────────────────────────────────────────────────

void DiagnosticsManager::set_provider(const DiagnosticsDataProvider* provider) {
    provider_ = provider;
}

// ── Profiler access ──────────────────────────────────────────────────────

Profiler& DiagnosticsManager::profiler() { return profiler_; }
const Profiler& DiagnosticsManager::profiler() const { return profiler_; }

// ── Snapshots ───────────────────────────────────────────────────────────

PerformanceSnapshot DiagnosticsManager::capture_snapshot(const std::string& label) {
    PerformanceSnapshot snap;
    snap.label = label;

    // Get current timestamp.
    auto now = std::chrono::steady_clock::now();
    snap.timestamp_ms = static_cast<std::uint64_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count());

    if (provider_) {
        snap.fps               = provider_->fps;
        snap.frame_time_ms     = provider_->frame_time_ms;
        snap.cpu_time_ms       = provider_->cpu_time_ms;
        snap.gpu_time_ms       = provider_->gpu_time_ms;
        snap.ram_used_bytes    = provider_->ram_used;
        snap.vram_used_bytes   = provider_->vram_used;
        snap.events_published  = provider_->events_published;
        snap.services_active   = provider_->services_active;
        snap.resources_active  = provider_->resources_active;
        snap.tasks_pending     = provider_->tasks_pending;
    }

    snapshots_.push_back(snap);

    std::ostringstream oss;
    oss << "DiagnosticsManager: snapshot captured — '" << label << "'";
    LIZ_INFO(oss.str());

    return snap;
}

const std::vector<PerformanceSnapshot>& DiagnosticsManager::snapshots() const {
    return snapshots_;
}

std::size_t DiagnosticsManager::snapshot_count() const {
    return snapshots_.size();
}

// ── Statistics ───────────────────────────────────────────────────────────

EngineStatistics DiagnosticsManager::statistics() const {
    EngineStatistics stats;

    if (provider_) {
        stats.total_runtime_ms    = provider_->total_runtime_ms;
        stats.frames_processed    = provider_->frames_processed;
        stats.events_published    = provider_->events_published;
        stats.services_registered = provider_->services_registered;
        stats.resources_loaded    = provider_->resources_loaded;
        stats.plugins_active      = provider_->plugins_active;
        stats.batches_executed    = provider_->batches_executed;
        stats.inferences_executed = provider_->inferences_executed;
        stats.gpu_commands        = provider_->gpu_commands;
        stats.assets_loaded        = provider_->assets_loaded;
        stats.assets_active        = provider_->assets_active;
        stats.pipelines_active     = provider_->pipelines_active;
        stats.pipeline_nodes       = provider_->pipeline_nodes;
        stats.pipeline_exec_time_ms = provider_->pipeline_exec_time_ms;
        stats.pipeline_errors      = provider_->pipeline_errors;
        stats.active_project       = provider_->active_project;
        stats.open_projects        = provider_->open_projects;
        stats.saved_projects       = provider_->saved_projects;
    }

    return stats;
}

// ── Reporting ────────────────────────────────────────────────────────────

void DiagnosticsManager::print_report() const {
    std::ostringstream oss;

    oss << std::endl;
    oss << "===== LIZ Diagnostics =====" << std::endl;

    // Profiler sessions.
    oss << "Profiler:" << std::endl;
    if (profiler_.session_count() == 0) {
        oss << "  (no sessions)" << std::endl;
    } else {
        for (const auto& session : profiler_.sessions()) {
            oss << "  " << session.info() << std::endl;
        }
    }

    // Statistics.
    auto stats = statistics();
    oss << std::endl;
    oss << "Engine:" << std::endl;
    oss << "  Runtime:       " << std::fixed << std::setprecision(1)
        << stats.total_runtime_ms << " ms" << std::endl;
    oss << "  Frames:        " << stats.frames_processed << std::endl;
    oss << "  Events:        " << stats.events_published << std::endl;
    oss << "  Services:      " << stats.services_registered << std::endl;
    oss << "  Resources:     " << stats.resources_loaded << std::endl;
    oss << "  Plugins:       " << stats.plugins_active << std::endl;
    oss << "  Batches:       " << stats.batches_executed << std::endl;
    oss << "  Inferences:    " << stats.inferences_executed << std::endl;
    oss << "  GPU Commands:  " << stats.gpu_commands << std::endl;
    oss << "  Assets:        " << stats.assets_loaded << " (" << stats.assets_active << " active)" << std::endl;
    oss << "  Pipelines:     " << stats.pipelines_active << " active (" << stats.pipeline_nodes << " nodes)" << std::endl;
    oss << "  Projects:      " << stats.active_project << " active (" << stats.open_projects << " open, " << stats.saved_projects << " saved)" << std::endl;

    // Latest snapshot performance metrics.
    oss << std::endl;
    oss << "Performance:" << std::endl;
    if (!snapshots_.empty()) {
        const auto& latest = snapshots_.back();
        oss << "  FPS:           " << std::fixed << std::setprecision(1)
            << latest.fps << std::endl;
        oss << "  Frame Time:    " << latest.frame_time_ms << " ms" << std::endl;
        oss << "  CPU Time:      " << latest.cpu_time_ms << " ms" << std::endl;
        oss << "  GPU Time:      " << latest.gpu_time_ms << " ms" << std::endl;
        oss << "  RAM:           " << (latest.ram_used_bytes / 1024) << " KB" << std::endl;
        oss << "  VRAM:          " << (latest.vram_used_bytes / (1024 * 1024)) << " MB" << std::endl;
    } else {
        oss << "  (no snapshots)" << std::endl;
    }

    oss << std::endl;

    // All snapshots.
    oss << "Snapshots:  " << snapshots_.size() << " captured" << std::endl;
    for (std::size_t i = 0; i < snapshots_.size(); ++i) {
        const auto& s = snapshots_[i];
        oss << "  [" << (i + 1) << "] '" << s.label << "'"
            << " fps=" << std::fixed << std::setprecision(1) << s.fps
            << " tasks=" << s.tasks_pending
            << " services=" << s.services_active
            << " resources=" << s.resources_active
            << std::endl;
    }

    oss << std::endl;
    oss << "===========================" << std::endl;

    // Log the report via both Logger and stdout for visibility.
    LIZ_INFO(oss.str());
    std::cout << oss.str();
}

// ── Reset ────────────────────────────────────────────────────────────────

void DiagnosticsManager::reset() {
    profiler_.reset();
    snapshots_.clear();
    LIZ_INFO("DiagnosticsManager: reset");
}

} // namespace liz
