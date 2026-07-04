#include "engine/pipeline/PipelineGraph.h"
#include "engine/core/Logger.h"
#include "engine/events/EventBus.h"
#include "engine/events/Event.h"
#include "engine/events/EventType.h"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <random>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

namespace liz {

// ── Constructor ───────────────────────────────────────────────────────

PipelineGraph::PipelineGraph()
    : uuid_(generate_uuid())
    , name_("unnamed") {}

PipelineGraph::PipelineGraph(std::string name)
    : uuid_(generate_uuid())
    , name_(std::move(name)) {}

// ── Identity ──────────────────────────────────────────────────────────

const std::string& PipelineGraph::uuid() const { return uuid_; }
const std::string& PipelineGraph::name() const { return name_; }

// ── Node management ────────────────────────────────────────────────────

PipelineNode* PipelineGraph::create_node(const std::string& name, PipelineNodeType type) {
    auto node = std::make_unique<PipelineNode>(name, type);
    auto* ptr = node.get();

    std::ostringstream oss;
    oss << "PipelineGraph: created node '" << name
        << "' type=" << pipeline_node_type_to_string(type)
        << " uuid=" << ptr->uuid();
    LIZ_INFO(oss.str());

    publish_event(EventType::NodeCreated,
        "PipelineGraph",
        "node='" + name + "' type=" + pipeline_node_type_to_string(type));

    nodes_.push_back(std::move(node));
    return ptr;
}

bool PipelineGraph::remove_node(const std::string& node_uuid) {
    // Find and remove the node.
    auto node_it = std::find_if(nodes_.begin(), nodes_.end(),
        [&node_uuid](const auto& n) { return n->uuid() == node_uuid; });

    if (node_it == nodes_.end()) return false;

    std::string node_name = (*node_it)->name();

    // Remove all edges connected to this node.
    edges_.erase(
        std::remove_if(edges_.begin(), edges_.end(),
            [&node_uuid](const auto& e) {
                return e->source_uuid() == node_uuid ||
                       e->dest_uuid() == node_uuid;
            }),
        edges_.end());

    // Remove the node itself.
    nodes_.erase(node_it);

    std::ostringstream oss;
    oss << "PipelineGraph: removed node '" << node_name << "'";
    LIZ_INFO(oss.str());

    return true;
}

PipelineNode* PipelineGraph::find_node(const std::string& node_uuid) const {
    for (const auto& node : nodes_) {
        if (node->uuid() == node_uuid) return node.get();
    }
    return nullptr;
}

PipelineNode* PipelineGraph::find_node_by_name(const std::string& name) const {
    for (const auto& node : nodes_) {
        if (node->name() == name) return node.get();
    }
    return nullptr;
}

// ── Edge management ───────────────────────────────────────────────────

bool PipelineGraph::connect(const std::string& source_uuid,
                              const std::string& dest_uuid,
                              ConnectionType type) {
    // Validate both nodes exist.
    if (!find_node(source_uuid) || !find_node(dest_uuid)) {
        LIZ_WARN("PipelineGraph: cannot connect — node(s) not found");
        return false;
    }

    // Prevent self-connection.
    if (source_uuid == dest_uuid) {
        LIZ_WARN("PipelineGraph: cannot connect node to itself");
        return false;
    }

    // Prevent duplicate edges.
    for (const auto& edge : edges_) {
        if (edge->source_uuid() == source_uuid &&
            edge->dest_uuid() == dest_uuid) {
            return true; // Already connected.
        }
    }

    // Check for cycles.
    if (would_create_cycle(source_uuid, dest_uuid)) {
        LIZ_WARN("PipelineGraph: connection rejected — would create a cycle");
        last_validation_ = false;
        return false;
    }

    auto edge = std::make_unique<PipelineEdge>(source_uuid, dest_uuid, type);

    // Update node connection tracking.
    auto* src_node = find_node(source_uuid);
    auto* dst_node = find_node(dest_uuid);
    if (src_node) src_node->add_output(dest_uuid);
    if (dst_node) dst_node->add_input(source_uuid);

    std::ostringstream oss;
    oss << "PipelineGraph: connected '"
        << (src_node ? src_node->name() : "?") << "' -> '"
        << (dst_node ? dst_node->name() : "?") << "'"
        << " type=" << connection_type_to_string(type);
    LIZ_INFO(oss.str());

    edges_.push_back(std::move(edge));
    last_validation_ = true;
    return true;
}

bool PipelineGraph::disconnect(const std::string& edge_uuid) {
    auto it = std::find_if(edges_.begin(), edges_.end(),
        [&edge_uuid](const auto& e) { return e->uuid() == edge_uuid; });

    if (it == edges_.end()) return false;

    // Clean up node connection tracking.
    auto* src_node = find_node((*it)->source_uuid());
    auto* dst_node = find_node((*it)->dest_uuid());
    if (src_node) src_node->remove_output((*it)->dest_uuid());
    if (dst_node) dst_node->remove_input((*it)->source_uuid());

    edges_.erase(it);

    LIZ_INFO("PipelineGraph: edge disconnected");
    return true;
}

bool PipelineGraph::disconnect_nodes(const std::string& source_uuid,
                                       const std::string& dest_uuid) {
    bool found = false;
    for (auto it = edges_.begin(); it != edges_.end(); ) {
        if ((*it)->source_uuid() == source_uuid &&
            (*it)->dest_uuid() == dest_uuid) {
            it = edges_.erase(it);
            found = true;
        } else {
            ++it;
        }
    }

    if (found) {
        auto* src_node = find_node(source_uuid);
        auto* dst_node = find_node(dest_uuid);
        if (src_node) src_node->remove_output(dest_uuid);
        if (dst_node) dst_node->remove_input(source_uuid);

        LIZ_INFO("PipelineGraph: nodes disconnected");
    }

    return found;
}

PipelineEdge* PipelineGraph::find_edge(const std::string& edge_uuid) const {
    for (const auto& edge : edges_) {
        if (edge->uuid() == edge_uuid) return edge.get();
    }
    return nullptr;
}

// ── Queries ──────────────────────────────────────────────────────────

std::vector<std::string> PipelineGraph::list_nodes() const {
    std::vector<std::string> result;
    for (const auto& node : nodes_) {
        result.push_back(node->uuid());
    }
    return result;
}

std::vector<std::string> PipelineGraph::list_edges() const {
    std::vector<std::string> result;
    for (const auto& edge : edges_) {
        result.push_back(edge->uuid());
    }
    return result;
}

std::size_t PipelineGraph::node_count() const { return nodes_.size(); }
std::size_t PipelineGraph::edge_count() const { return edges_.size(); }

// ── Validation ────────────────────────────────────────────────────────

bool PipelineGraph::validate() const {
    // Check for cycles from every node using DFS.
    for (const auto& node : nodes_) {
        if (has_cycle_from(node->uuid())) {
            std::ostringstream oss;
            oss << "PipelineGraph: validation FAILED — cycle detected from '"
                << node->name() << "'";
            LIZ_WARN(oss.str());
            last_validation_ = false;
            return false;
        }
    }

    last_validation_ = true;
    return true;
}

bool PipelineGraph::last_validation_result() const {
    return last_validation_;
}

// ── Statistics ─────────────────────────────────────────────────────────

PipelineStatistics PipelineGraph::statistics() const {
    PipelineStatistics stats;
    stats.total_nodes = nodes_.size();
    stats.total_edges = edges_.size();
    stats.validation_result = last_validation_;

    for (const auto& node : nodes_) {
        switch (node->state()) {
            case PipelineNodeState::Completed: stats.nodes_executed++; break;
            case PipelineNodeState::Failed:    stats.nodes_failed++; break;
            case PipelineNodeState::Disabled:  stats.nodes_disabled++; break;
            default: break;
        }
    }

    return stats;
}

// ── Bulk operations ─────────────────────────────────────────────────────

void PipelineGraph::clear() {
    nodes_.clear();
    edges_.clear();
    last_validation_ = true;
    LIZ_INFO("PipelineGraph: cleared all nodes and edges");
}

// ── EventBus ───────────────────────────────────────────────────────────

void PipelineGraph::set_event_bus(EventBus* bus) {
    event_bus_ = bus;
}

// ── Cycle detection ────────────────────────────────────────────────────

bool PipelineGraph::would_create_cycle(const std::string& source_uuid,
                                        const std::string& dest_uuid) const {
    // Temporarily add the edge and check.
    // If dest_uuid can reach source_uuid through existing edges,
    // adding source->dest would create a cycle.

    // Build adjacency list.
    std::unordered_map<std::string, std::vector<std::string>> adj;
    for (const auto& edge : edges_) {
        adj[edge->source_uuid()].push_back(edge->dest_uuid());
    }

    // Add the proposed edge.
    adj[source_uuid].push_back(dest_uuid);

    // Check if dest_uuid can reach source_uuid (DFS from dest_uuid).
    std::unordered_set<std::string> visited;
    std::vector<std::string> stack;
    stack.push_back(dest_uuid);

    while (!stack.empty()) {
        auto current = stack.back();
        stack.pop_back();

        if (current == source_uuid) {
            return true; // Cycle detected.
        }

        if (visited.count(current)) continue;
        visited.insert(current);

        auto it = adj.find(current);
        if (it != adj.end()) {
            for (const auto& next : it->second) {
                if (!visited.count(next)) {
                    stack.push_back(next);
                }
            }
        }
    }

    return false;
}

bool PipelineGraph::has_cycle_from(const std::string& start_uuid) const {
    std::unordered_set<std::string> visited;
    std::unordered_set<std::string> in_stack;
    std::vector<std::string> stack;

    stack.push_back(start_uuid);

    while (!stack.empty()) {
        auto current = stack.back();
        stack.pop_back();

        if (in_stack.count(current)) {
            return true; // Back edge = cycle.
        }

        if (visited.count(current)) continue;
        visited.insert(current);
        in_stack.insert(current);

        for (const auto& edge : edges_) {
            if (edge->source_uuid() == current) {
                if (!visited.count(edge->dest_uuid())) {
                    stack.push_back(edge->dest_uuid());
                }
            }
        }

        in_stack.erase(current);
    }

    return false;
}

// ── Helpers ────────────────────────────────────────────────────────────

void PipelineGraph::publish_event(EventType type, const std::string& source, const std::string& message) {
    if (event_bus_) {
        Event event(type, source, message);
        event_bus_->publish(event);
    }
}

std::string PipelineGraph::generate_uuid() {
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
