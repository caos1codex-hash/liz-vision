#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <random>
#include <string>

namespace liz {

/// Represents a completed profiling session.
///
/// Contains timing metadata: UUID, name, start/end timestamps,
/// total duration, and the number of sections (samples) measured.
class ProfilerSession {
public:
    ProfilerSession();
    ProfilerSession(std::string name,
                    std::chrono::steady_clock::time_point start,
                    std::chrono::steady_clock::time_point end,
                    std::size_t sample_count);

    ~ProfilerSession() = default;

    // Non-copyable.
    ProfilerSession(const ProfilerSession&) = delete;
    ProfilerSession& operator=(const ProfilerSession&) = delete;

    // Movable.
    ProfilerSession(ProfilerSession&&) noexcept = default;
    ProfilerSession& operator=(ProfilerSession&&) noexcept = default;

    // ── Accessors ───────────────────────────────────────────────────────

    /// Unique session identifier.
    const std::string& uuid() const;

    /// Human-readable session name.
    const std::string& name() const;

    /// Session start timestamp (epoch ms).
    std::uint64_t start_timestamp_ms() const;

    /// Session end timestamp (epoch ms).
    std::uint64_t end_timestamp_ms() const;

    /// Total duration in milliseconds.
    double duration_ms() const;

    /// Number of sections (samples) measured during this session.
    std::size_t sample_count() const;

    /// Generate a summary string for logging.
    std::string info() const;

private:
    /// Generate a short random UUID (8 hex chars).
    static std::string generate_uuid();

    std::string  uuid_;
    std::string  name_;
    std::chrono::steady_clock::time_point start_;
    std::chrono::steady_clock::time_point end_;
    std::size_t  sample_count_ = 0;
};

} // namespace liz
