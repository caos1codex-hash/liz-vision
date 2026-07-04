#include "engine/diagnostics/Profiler.h"
#include "engine/core/Logger.h"

#include <algorithm>
#include <chrono>
#include <sstream>

namespace liz {

// ── Lifecycle ────────────────────────────────────────────────────────────

void Profiler::start(const std::string& name) {
    if (running_) {
        LIZ_WARN("Profiler: already running — stop current session first");
        return;
    }

    running_ = true;
    start_time_ = std::chrono::steady_clock::now();
    current_session_name_ = name;
    sections_.clear();

    std::ostringstream oss;
    oss << "Profiler: started session '" << name << "'";
    LIZ_INFO(oss.str());
}

void Profiler::stop() {
    if (!running_) {
        LIZ_WARN("Profiler: not running — nothing to stop");
        return;
    }

    auto end_time = std::chrono::steady_clock::now();
    running_ = false;

    ProfilerSession session(
        current_session_name_,
        start_time_,
        end_time,
        sections_.size());

    std::ostringstream oss;
    oss << "Profiler: stopped session '" << current_session_name_
        << "' — " << session.duration_ms() << "ms, "
        << sections_.size() << " sections";
    LIZ_INFO(oss.str());

    completed_sessions_.push_back(std::move(session));
}

bool Profiler::is_running() const {
    return running_;
}

// ── Sections ─────────────────────────────────────────────────────────────

void Profiler::begin_section(const std::string& section_name) {
    if (!running_) {
        LIZ_WARN("Profiler: cannot begin section — not running");
        return;
    }

    auto now = std::chrono::steady_clock::now();

    // Store the start timestamp in a SectionRecord.
    // We use duration_ms as a sentinel: -1.0 means "in progress".
    SectionRecord rec;
    rec.name = section_name;
    rec.duration_ms = -1.0;

    // Store the start time in a side channel.
    // We use the sections_ vector and a parallel vector for start times.
    // To keep it simple, we store the start time as a negative epoch-ms value.
    auto epoch_ms = std::chrono::duration<double, std::milli>(
        now.time_since_epoch()).count();
    rec.duration_ms = -epoch_ms;  // negative = start timestamp stored

    sections_.push_back(rec);
}

void Profiler::end_section(const std::string& section_name) {
    if (!running_) {
        LIZ_WARN("Profiler: cannot end section — not running");
        return;
    }

    auto now = std::chrono::steady_clock::now();
    auto epoch_ms = std::chrono::duration<double, std::milli>(
        now.time_since_epoch()).count();

    // Find the last section with this name that is still "in progress".
    for (auto it = sections_.rbegin(); it != sections_.rend(); ++it) {
        if (it->name == section_name && it->duration_ms < 0) {
            double start_epoch = -it->duration_ms;
            it->duration_ms = epoch_ms - start_epoch;

            std::ostringstream oss;
            oss << "Profiler: section '" << section_name
                << "' = " << it->duration_ms << "ms";
            LIZ_DEBUG(oss.str());
            return;
        }
    }

    std::ostringstream oss;
    oss << "Profiler: end_section('" << section_name
        << "') — no matching begin_section found";
    LIZ_WARN(oss.str());
}

double Profiler::elapsed_ms() const {
    if (!running_) {
        return 0.0;
    }
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration<double, std::milli>(
        now - start_time_).count();
}

// ── Sessions ──────────────────────────────────────────────────────────────

const std::vector<ProfilerSession>& Profiler::sessions() const {
    return completed_sessions_;
}

std::size_t Profiler::session_count() const {
    return completed_sessions_.size();
}

// ── Reset ─────────────────────────────────────────────────────────────────

void Profiler::reset() {
    running_ = false;
    sections_.clear();
    completed_sessions_.clear();
    current_session_name_.clear();
    LIZ_INFO("Profiler: reset");
}

} // namespace liz
