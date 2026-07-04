#include "engine/pipeline/PipelineEdge.h"
#include "engine/core/Logger.h"

#include <cstdint>
#include <chrono>
#include <random>
#include <sstream>

namespace liz {

// ── Constructor ───────────────────────────────────────────────────────

PipelineEdge::PipelineEdge(std::string source_uuid, std::string dest_uuid,
                           ConnectionType type)
    : uuid_(generate_uuid())
    , source_uuid_(std::move(source_uuid))
    , dest_uuid_(std::move(dest_uuid))
    , type_(type) {}

// ── Identity ──────────────────────────────────────────────────────────

const std::string& PipelineEdge::uuid() const { return uuid_; }
const std::string& PipelineEdge::source_uuid() const { return source_uuid_; }
const std::string& PipelineEdge::dest_uuid() const { return dest_uuid_; }

// ── Type ─────────────────────────────────────────────────────────────

ConnectionType PipelineEdge::type() const { return type_; }

void PipelineEdge::set_type(ConnectionType type) {
    type_ = type;
}

// ── State ────────────────────────────────────────────────────────────

bool PipelineEdge::is_active() const { return active_; }

void PipelineEdge::activate() {
    active_ = true;
}

void PipelineEdge::deactivate() {
    active_ = false;
}

// ── Info ───────────────────────────────────────────────────────────────

std::string PipelineEdge::info() const {
    std::ostringstream oss;
    oss << "Edge{src=" << source_uuid_
        << " -> dst=" << dest_uuid_
        << " type=" << connection_type_to_string(type_)
        << " active=" << (active_ ? "true" : "false")
        << "}";
    return oss.str();
}

// ── UUID generation ───────────────────────────────────────────────────

std::string PipelineEdge::generate_uuid() {
    auto seed = static_cast<std::uint64_t>(
        std::chrono::steady_clock::now().time_since_epoch().count());
    std::mt19937_64 rng(seed);
    std::uniform_int_distribution<std::uint64_t> dist(0, 0xFFFF'FFFF);

    static constexpr int kLen = 8;
    char buf[kLen + 1];
    for (int i = 0; i < kLen; ++i) {
        std::snprintf(buf + i, 2, "%x", static_cast<int>(dist(rng) & 0xF));
    }
    buf[kLen] = '\0';
    return buf;
}

} // namespace liz
