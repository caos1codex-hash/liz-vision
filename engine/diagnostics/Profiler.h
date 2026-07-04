#pragma once

#include "engine/diagnostics/ProfilerSession.h"

#include <chrono>
#include <cstddef>
#include <string>
#include <vector>

namespace liz {

/// Lightweight profiler for measuring elapsed time and section durations.
///
/// Usage:
///   Profiler profiler;
///   profiler.start("my_session");
///   profiler.begin_section("decode");
///   // ... do work ...
///   profiler.end_section("decode");
///   profiler.stop();
///   double total_ms = profiler.elapsed_ms();
class Profiler {
public:
    Profiler() = default;
    ~Profiler() = default;

    // Non-copyable.
    Profiler(const Profiler&) = delete;
    Profiler& operator=(const Profiler&) = delete;

    // ── Lifecycle ────────────────────────────────────────────────────────

    /// Start a new profiling session with the given name.
    /// Creates a ProfilerSession internally.
    void start(const std::string& name);

    /// Stop the current profiling session.
    /// Finalizes timing and stores the completed session.
    void stop();

    /// Check if the profiler is currently running.
    bool is_running() const;

    // ── Sections ─────────────────────────────────────────────────────────

    /// Begin a named section (records start time).
    void begin_section(const std::string& section_name);

    /// End a named section (records end time and duration).
    void end_section(const std::string& section_name);

    /// Total elapsed time in milliseconds since start().
    double elapsed_ms() const;

    // ── Sessions ──────────────────────────────────────────────────────────

    /// Get all completed sessions.
    const std::vector<ProfilerSession>& sessions() const;

    /// Number of completed sessions.
    std::size_t session_count() const;

    // ── Reset ───────────────────────────────────────────────────────────

    /// Reset the profiler (clear all sessions and state).
    void reset();

private:
    bool running_ = false;
    std::chrono::steady_clock::time_point start_time_;
    std::string current_session_name_;

    // Section tracking: name -> start time.
    struct SectionRecord {
        std::string name;
        double duration_ms = 0.0;
    };
    std::vector<SectionRecord> sections_;

    std::vector<ProfilerSession> completed_sessions_;
};

} // namespace liz
