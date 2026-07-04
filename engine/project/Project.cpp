#include "engine/project/Project.h"

#include <cstdint>
#include <cstdio>
#include <random>
#include <sstream>

namespace liz {

// ── Constructor ────────────────────────────────────────────────────

Project::Project(std::string name)
    : uuid_(generate_uuid())
    , name_(std::move(name))
    , state_(ProjectState::Created)
    , created_at_(std::chrono::steady_clock::now())
    , last_modified_(created_at_) {}

// ── UUID generation ─────────────────────────────────────────────────

std::string Project::generate_uuid() {
    auto seed = static_cast<std::uint64_t>(
        std::chrono::steady_clock::now().time_since_epoch().count());
    std::mt19937_64 rng(seed);
    std::uniform_int_distribution<std::uint64_t> dist(0, 0xFFFF'FFFF);

    static constexpr int kIdLength = 8;
    char buf[kIdLength + 1];
    for (int i = 0; i < kIdLength; ++i) {
        std::snprintf(buf + i, 2, "%x", static_cast<int>(dist(rng) & 0xF));
    }
    buf[kIdLength] = '\0';
    return buf;
}

// ── Info ──────────────────────────────────────────────────────────

std::string Project::to_string() const {
    std::ostringstream oss;
    oss << "Project{"
        << " uuid=" << uuid_
        << " name=" << name_
        << " state=" << project_state_to_string(state_)
        << " author=" << author_
        << " version=" << version_
        << " }";
    return oss.str();
}

} // namespace liz
