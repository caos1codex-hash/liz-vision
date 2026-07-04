#include "engine/diagnostics/ProfilerSession.h"

#include <chrono>
#include <cstdint>
#include <random>
#include <sstream>

namespace liz {

// ── Constructors ──────────────────────────────────────────────────────────

ProfilerSession::ProfilerSession()
    : uuid_(generate_uuid())
    , name_("Unnamed")
    , sample_count_(0) {}

ProfilerSession::ProfilerSession(std::string name,
                                  std::chrono::steady_clock::time_point start,
                                  std::chrono::steady_clock::time_point end,
                                  std::size_t sample_count)
    : uuid_(generate_uuid())
    , name_(std::move(name))
    , start_(start)
    , end_(end)
    , sample_count_(sample_count) {}

// ── Accessors ────────────────────────────────────────────────────────────

const std::string& ProfilerSession::uuid() const { return uuid_; }
const std::string& ProfilerSession::name() const { return name_; }

std::uint64_t ProfilerSession::start_timestamp_ms() const {
    return static_cast<std::uint64_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            start_.time_since_epoch()).count());
}

std::uint64_t ProfilerSession::end_timestamp_ms() const {
    return static_cast<std::uint64_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            end_.time_since_epoch()).count());
}

double ProfilerSession::duration_ms() const {
    return std::chrono::duration<double, std::milli>(end_ - start_).count();
}

std::size_t ProfilerSession::sample_count() const { return sample_count_; }

std::string ProfilerSession::info() const {
    std::ostringstream oss;
    oss << "ProfilerSession[" << name_
        << "] uuid=" << uuid_
        << " duration=" << duration_ms() << "ms"
        << " samples=" << sample_count_;
    return oss.str();
}

// ── Private ──────────────────────────────────────────────────────────────

std::string ProfilerSession::generate_uuid() {
    static constexpr int kLength = 8;

    auto seed = static_cast<std::uint64_t>(
        std::chrono::steady_clock::now().time_since_epoch().count());
    std::mt19937_64 rng(seed);
    std::uniform_int_distribution<std::uint64_t> dist(0, 0xFFFF'FFFF);

    char buf[kLength + 1];
    for (int i = 0; i < kLength; ++i) {
        std::snprintf(buf + i, 2, "%x", static_cast<int>(dist(rng) & 0xF));
    }
    buf[kLength] = '\0';
    return std::string(buf);
}

} // namespace liz
