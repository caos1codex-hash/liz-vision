#include "engine/pipeline/PipelineNode.h"
#include "engine/core/Logger.h"

#include <cstdint>
#include <chrono>
#include <random>
#include <sstream>

namespace liz {

// ── Constructor ───────────────────────────────────────────────────────

PipelineNode::PipelineNode(std::string name, PipelineNodeType type)
    : uuid_(generate_uuid())
    , name_(std::move(name))
    , type_(type) {}

// ── Identity ──────────────────────────────────────────────────────────

const std::string& PipelineNode::uuid() const { return uuid_; }
const std::string& PipelineNode::name() const { return name_; }
PipelineNodeType PipelineNode::type() const { return type_; }

// ── State ─────────────────────────────────────────────────────────────

PipelineNodeState PipelineNode::state() const { return state_; }

void PipelineNode::set_state(PipelineNodeState state) {
    state_ = state;
}

bool PipelineNode::is_enabled() const { return enabled_; }

void PipelineNode::enable() {
    enabled_ = true;
    if (state_ == PipelineNodeState::Disabled) {
        state_ = PipelineNodeState::Created;
    }
    std::ostringstream oss;
    oss << "PipelineNode: '" << name_ << "' enabled";
    LIZ_INFO(oss.str());
}

void PipelineNode::disable() {
    enabled_ = false;
    state_ = PipelineNodeState::Disabled;
    std::ostringstream oss;
    oss << "PipelineNode: '" << name_ << "' disabled";
    LIZ_INFO(oss.str());
}

bool PipelineNode::is_ready() const {
    return enabled_ &&
           (state_ == PipelineNodeState::Created ||
            state_ == PipelineNodeState::Ready);
}

// ── Connections ───────────────────────────────────────────────────────

const std::vector<std::string>& PipelineNode::inputs() const { return inputs_; }

void PipelineNode::add_input(const std::string& node_uuid) {
    // Avoid duplicates.
    for (const auto& id : inputs_) {
        if (id == node_uuid) return;
    }
    inputs_.push_back(node_uuid);
}

void PipelineNode::remove_input(const std::string& node_uuid) {
    auto it = std::find(inputs_.begin(), inputs_.end(), node_uuid);
    if (it != inputs_.end()) {
        inputs_.erase(it);
    }
}

const std::vector<std::string>& PipelineNode::outputs() const { return outputs_; }

void PipelineNode::add_output(const std::string& node_uuid) {
    for (const auto& id : outputs_) {
        if (id == node_uuid) return;
    }
    outputs_.push_back(node_uuid);
}

void PipelineNode::remove_output(const std::string& node_uuid) {
    auto it = std::find(outputs_.begin(), outputs_.end(), node_uuid);
    if (it != outputs_.end()) {
        outputs_.erase(it);
    }
}

// ── Position ──────────────────────────────────────────────────────────

std::int32_t PipelineNode::x() const { return x_; }
std::int32_t PipelineNode::y() const { return y_; }

void PipelineNode::set_position(std::int32_t x, std::int32_t y) {
    x_ = x;
    y_ = y;
}

// ── Execution ─────────────────────────────────────────────────────────

bool PipelineNode::execute() {
    if (!enabled_) {
        std::ostringstream oss;
        oss << "PipelineNode: '" << name_ << "' skipped (disabled)";
        LIZ_INFO(oss.str());
        return true;
    }

    state_ = PipelineNodeState::Running;

    std::ostringstream oss;
    oss << "PipelineNode: executing '" << name_
        << "' type=" << pipeline_node_type_to_string(type_);
    LIZ_INFO(oss.str());

    // Simulated execution — always succeeds.
    // In future sprints, this will delegate to real processing.
    state_ = PipelineNodeState::Completed;

    std::ostringstream oss2;
    oss2 << "PipelineNode: '" << name_ << "' completed";
    LIZ_INFO(oss2.str());

    return true;
}

// ── Info ───────────────────────────────────────────────────────────────

std::string PipelineNode::info() const {
    std::ostringstream oss;
    oss << "Node{name='" << name_
        << "' type=" << pipeline_node_type_to_string(type_)
        << " state=" << pipeline_node_state_to_string(state_)
        << " inputs=" << inputs_.size()
        << " outputs=" << outputs_.size()
        << " pos=(" << x_ << "," << y_ << ")"
        << " enabled=" << (enabled_ ? "true" : "false")
        << "}";
    return oss.str();
}

// ── UUID generation ───────────────────────────────────────────────────

std::string PipelineNode::generate_uuid() {
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
