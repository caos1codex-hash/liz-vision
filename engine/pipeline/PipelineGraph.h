#pragma once

#include "engine/pipeline/PipelineNode.h"
#include "engine/pipeline/PipelineEdge.h"
#include "engine/pipeline/PipelineStatistics.h"
#include "engine/pipeline/PipelineTypes.h"
#include "engine/events/EventType.h"

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

namespace liz {

class EventBus;

/// A directed acyclic graph representing a processing pipeline.
///
/// Nodes represent operations (input, decode, filter, inference, output).
/// Edges represent the flow of data between nodes.
///
/// The graph validates against cycles before execution.
/// All processing is simulated — no real AI/GPU work is performed.
class PipelineGraph {
public:
    PipelineGraph();
    explicit PipelineGraph(std::string name);
    ~PipelineGraph() = default;

    // Non-copyable.
    PipelineGraph(const PipelineGraph&) = delete;
    PipelineGraph& operator=(const PipelineGraph&) = delete;

    // Movable.
    PipelineGraph(PipelineGraph&&) noexcept = default;
    PipelineGraph& operator=(PipelineGraph&&) noexcept = default;

    // ── Identity ─────────────────────────────────────────────────────

    const std::string& uuid() const;
    const std::string& name() const;

    // ── Node management ───────────────────────────────────────────────

    /// Create a node and add it to the graph.  Returns a pointer to the node.
    PipelineNode* create_node(const std::string& name, PipelineNodeType type);

    /// Remove a node by UUID.  Also removes all connected edges.
    bool remove_node(const std::string& node_uuid);

    /// Find a node by UUID.  Returns nullptr if not found.
    PipelineNode* find_node(const std::string& node_uuid) const;

    /// Find a node by name.  Returns nullptr if not found.
    PipelineNode* find_node_by_name(const std::string& name) const;

    // ── Edge management ───────────────────────────────────────────────

    /// Connect two nodes.  Returns true on success, false if it would create a cycle.
    bool connect(const std::string& source_uuid, const std::string& dest_uuid,
                 ConnectionType type = ConnectionType::Data);

    /// Disconnect an edge by UUID.
    bool disconnect(const std::string& edge_uuid);

    /// Disconnect all edges from a specific source to a specific destination.
    bool disconnect_nodes(const std::string& source_uuid, const std::string& dest_uuid);

    /// Find an edge by UUID.  Returns nullptr if not found.
    PipelineEdge* find_edge(const std::string& edge_uuid) const;

    // ── Queries ──────────────────────────────────────────────────────

    /// Get all node UUIDs.
    std::vector<std::string> list_nodes() const;

    /// Get all edge UUIDs.
    std::vector<std::string> list_edges() const;

    /// Number of nodes.
    std::size_t node_count() const;

    /// Number of edges.
    std::size_t edge_count() const;

    // ── Validation ────────────────────────────────────────────────────

    /// Validate the graph.  Checks for cycles using DFS.
    /// Returns true if the graph is a valid DAG.
    bool validate() const;

    /// Get the result of the last validation.
    bool last_validation_result() const;

    // ── Statistics ──────────────────────────────────────────────────

    PipelineStatistics statistics() const;

    // ── Bulk operations ──────────────────────────────────────────────

    /// Remove all nodes and edges.
    void clear();

    // ── EventBus ──────────────────────────────────────────────────────

    void set_event_bus(EventBus* bus);

private:
    // ── Cycle detection ──────────────────────────────────────────────
    /// Returns true if adding source->dest would create a cycle.
    bool would_create_cycle(const std::string& source_uuid,
                            const std::string& dest_uuid) const;

    /// DFS-based cycle detection from a given start node.
    bool has_cycle_from(const std::string& start_uuid) const;

    // ── Helpers ────────────────────────────────────────────────────────
    void publish_event(EventType type, const std::string& source, const std::string& message);

    static std::string generate_uuid();

    std::string    uuid_;
    std::string    name_;
    EventBus*      event_bus_ = nullptr;

    std::vector<std::unique_ptr<PipelineNode>> nodes_;
    std::vector<std::unique_ptr<PipelineEdge>> edges_;

    mutable bool last_validation_ = true;
};

} // namespace liz
