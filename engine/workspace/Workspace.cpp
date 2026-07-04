#include "engine/workspace/Workspace.h"
#include "engine/workspace/WorkspaceTypes.h"

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <random>
#include <sstream>

namespace liz {

// ── Constructor ────────────────────────────────────────────────────

Workspace::Workspace(std::string name)
    : uuid_(generate_uuid())
    , name_(std::move(name))
    , state_(WorkspaceState::Created)
    , created_at_(std::chrono::steady_clock::now())
    , last_modified_(created_at_) {}

// ── Project management ────────────────────────────────────────────

bool Workspace::remove_project(const std::string& project_uuid) {
    auto it = std::find(project_uuids_.begin(), project_uuids_.end(), project_uuid);
    if (it == project_uuids_.end()) return false;
    project_uuids_.erase(it);
    return true;
}

// ── UUID generation ─────────────────────────────────────────────────

std::string Workspace::generate_uuid() {
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

std::string Workspace::to_string() const {
    std::ostringstream oss;
    oss << "Workspace{"
        << " uuid=" << uuid_
        << " name=" << name_
        << " state=" << workspace_state_to_string(state_)
        << " projects=" << project_uuids_.size()
        << " }";
    return oss.str();
}

} // namespace liz
