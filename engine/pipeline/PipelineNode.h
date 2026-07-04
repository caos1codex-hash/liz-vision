#pragma once

#include "engine/pipeline/PipelineTypes.h"

#include <cstdint>
#include <string>
#include <vector>

namespace liz {

/// A node in a pipeline graph.
///
/// Represents a single operation (input, decode, filter, inference, etc.)
/// in a directed acyclic graph.  Execution is simulated — no real processing
/// is performed in this sprint.
class PipelineNode {
public:
    PipelineNode(std::string name, PipelineNodeType type);

    ~PipelineNode() = default;

    // Non-copyable.
    PipelineNode(const PipelineNode&) = delete;
    PipelineNode& operator=(const PipelineNode&) = delete;

    // Movable.
    PipelineNode(PipelineNode&&) noexcept = default;
    PipelineNode& operator=(PipelineNode&&) noexcept = default;

    // ── Identity ─────────────────────────────────────────────────────

    const std::string& uuid() const;
    const std::string& name() const;
    PipelineNodeType   type() const;

    // ── State ─────────────────────────────────────────────────────────

    PipelineNodeState state() const;
    void              set_state(PipelineNodeState state);

    bool is_enabled() const;
    void enable();
    void disable();

    bool is_ready() const;

    // ── Connections ────────────────────────────────────────────────────

    /// UUIDs of nodes that feed into this node (inputs).
    const std::vector<std::string>& inputs() const;
    void add_input(const std::string& node_uuid);
    void remove_input(const std::string& node_uuid);

    /// UUIDs of nodes that this node feeds into (outputs).
    const std::vector<std::string>& outputs() const;
    void add_output(const std::string& node_uuid);
    void remove_output(const std::string& node_uuid);

    // ── Position ────────────────────────────────────────────────────

    /// Logical position (x, y) for visualization.
    std::int32_t x() const;
    std::int32_t y() const;
    void set_position(std::int32_t x, std::int32_t y);

    // ── Execution ────────────────────────────────────────────────────

    /// Simulated execution.  Returns true on success.
    bool execute();

    // ── Info ─────────────────────────────────────────────────────────

    /// One-line summary for logging.
    std::string info() const;

private:
    static std::string generate_uuid();

    std::string            uuid_;
    std::string            name_;
    PipelineNodeType       type_     = PipelineNodeType::Unknown;
    PipelineNodeState      state_    = PipelineNodeState::Created;
    bool                   enabled_  = true;
    std::vector<std::string> inputs_;
    std::vector<std::string> outputs_;
    std::int32_t           x_ = 0;
    std::int32_t           y_ = 0;
};

} // namespace liz
