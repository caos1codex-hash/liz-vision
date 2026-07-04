#include "engine/pipeline/PipelineExecutor.h"
#include "engine/core/Logger.h"
#include "engine/events/EventBus.h"
#include "engine/events/Event.h"
#include "engine/events/EventType.h"

#include <algorithm>
#include <chrono>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <queue>

namespace liz {

// ── Constructor ───────────────────────────────────────────────────────

PipelineExecutor::PipelineExecutor() = default;

// ── Lifecycle ─────────────────────────────────────────────────────────

bool PipelineExecutor::initialize() {
    if (initialized_) {
        LIZ_WARN("PipelineExecutor: already initialized");
        return true;
    }

    LIZ_INFO("PipelineExecutor: initializing");
    initialized_ = true;
    return true;
}

void PipelineExecutor::shutdown() {
    if (!initialized_) return;

    LIZ_INFO("PipelineExecutor: shutting down");
    clear();
    initialized_ = false;
}

// ── Pipeline management ───────────────────────────────────────────────

bool PipelineExecutor::register_pipeline(std::unique_ptr<PipelineGraph> graph) {
    if (!graph) return false;

    const auto& name = graph->name();

    // Check for duplicates.
    auto it = std::find_if(entries_.begin(), entries_.end(),
        [&name](const Entry& e) { return e.graph->name() == name; });

    if (it != entries_.end()) {
        std::ostringstream oss;
        oss << "PipelineExecutor: pipeline '" << name << "' already registered";
        LIZ_WARN(oss.str());
        return false;
    }

    std::ostringstream oss;
    oss << "PipelineExecutor: registered pipeline '" << name << "'"
        << " (" << graph->node_count() << " nodes, "
        << graph->edge_count() << " edges)";
    LIZ_INFO(oss.str());

    publish_event(EventType::PipelineCreated, "PipelineExecutor", "pipeline='" + name + "'");

    entries_.push_back(Entry{std::move(graph)});
    return true;
}

bool PipelineExecutor::destroy_pipeline(const std::string& name) {
    auto it = std::find_if(entries_.begin(), entries_.end(),
        [&name](const Entry& e) { return e.graph->name() == name; });

    if (it == entries_.end()) return false;

    it->graph.reset();
    entries_.erase(it);

    std::ostringstream oss;
    oss << "PipelineExecutor: destroyed pipeline '" << name << "'";
    LIZ_INFO(oss.str());

    publish_event(EventType::PipelineDestroyed, "PipelineExecutor", "pipeline='" + name + "'");
    return true;
}

PipelineGraph* PipelineExecutor::find_pipeline(const std::string& name) const {
    for (const auto& entry : entries_) {
        if (entry.graph->name() == name) return entry.graph.get();
    }
    return nullptr;
}

std::vector<std::string> PipelineExecutor::list_pipelines() const {
    std::vector<std::string> names;
    for (const auto& entry : entries_) {
        names.push_back(entry.graph->name());
    }
    return names;
}

std::size_t PipelineExecutor::pipeline_count() const {
    return entries_.size();
}

// ── Execution ─────────────────────────────────────────────────────────

PipelineStatistics PipelineExecutor::execute(const std::string& name) {
    PipelineStatistics stats;

    auto* graph = find_pipeline(name);
    if (!graph) {
        LIZ_ERROR("PipelineExecutor: pipeline '" + name + "' not found");
        return stats;
    }

    std::ostringstream oss;
    oss << "PipelineExecutor: executing pipeline '" << name << "'";
    LIZ_INFO(oss.str());

    // 1. Validate.
    if (!graph->validate()) {
        LIZ_ERROR("PipelineExecutor: pipeline '" + name + "' validation failed");
        stats.validation_result = false;
        total_failures_++;
        return stats;
    }

    // 2. Compute topological order.
    std::vector<std::string> order;
    if (!topological_sort(*graph, order)) {
        LIZ_ERROR("PipelineExecutor: pipeline '" + name + "' cycle detected during execution");
        stats.validation_result = false;
        total_failures_++;
        return stats;
    }

    std::ostringstream oss2;
    oss2 << "PipelineExecutor: execution order (";
    for (std::size_t i = 0; i < order.size(); ++i) {
        auto* node = graph->find_node(order[i]);
        if (node) {
            oss2 << (i > 0 ? " -> " : "") << node->name();
        }
    }
    oss2 << ")";
    LIZ_INFO(oss2.str());

    // 3. Execute nodes in order.
    auto start_time = std::chrono::steady_clock::now();

    for (const auto& node_uuid : order) {
        auto* node = graph->find_node(node_uuid);
        if (!node) continue;

        if (!node->is_enabled()) {
            stats.nodes_disabled++;
            continue;
        }

        node->set_state(PipelineNodeState::Ready);

        publish_event(EventType::NodeExecuted,
            "PipelineExecutor",
            "node='" + node->name() + "' pipeline='" + name + "'");

        bool success = node->execute();
        if (success) {
            stats.nodes_executed++;
        } else {
            stats.nodes_failed++;
        }
    }

    auto end_time = std::chrono::steady_clock::now();
    stats.execution_time_ms = static_cast<double>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            end_time - start_time).count());

    stats.total_nodes = graph->node_count();
    stats.total_edges = graph->edge_count();
    stats.validation_result = true;

    total_executions_++;

    publish_event(EventType::PipelineFinished, "PipelineExecutor", "pipeline='" + name + "'");

    std::ostringstream oss3;
    oss3 << "PipelineExecutor: pipeline '" << name << "' completed in "
         << stats.execution_time_ms << "ms";
    LIZ_INFO(oss3.str());

    return stats;
}

void PipelineExecutor::execute_all() {
    for (const auto& entry : entries_) {
        execute(entry.graph->name());
    }
}

// ── Statistics ─────────────────────────────────────────────────────────

PipelineStatistics PipelineExecutor::pipeline_statistics(const std::string& name) const {
    auto* graph = find_pipeline(name);
    if (!graph) return PipelineStatistics{};
    return graph->statistics();
}

PipelineStatistics PipelineExecutor::global_statistics() const {
    PipelineStatistics stats;
    for (const auto& entry : entries_) {
        auto ps = entry.graph->statistics();
        stats.total_nodes      += ps.total_nodes;
        stats.total_edges      += ps.total_edges;
        stats.nodes_executed   += ps.nodes_executed;
        stats.nodes_failed     += ps.nodes_failed;
        stats.nodes_disabled   += ps.nodes_disabled;
        stats.execution_time_ms += ps.execution_time_ms;
    }
    stats.validation_result = true;
    return stats;
}

// ── EventBus ───────────────────────────────────────────────────────────

void PipelineExecutor::set_event_bus(EventBus* bus) {
    event_bus_ = bus;
    // Propagate to all registered graphs.
    for (auto& entry : entries_) {
        entry.graph->set_event_bus(bus);
    }
}

// ── Bulk ──────────────────────────────────────────────────────────────

void PipelineExecutor::clear() {
    entries_.clear();
    LIZ_INFO("PipelineExecutor: all pipelines cleared");
}

// ── Topological sort (Kahn's algorithm) ───────────────────────────────

bool PipelineExecutor::topological_sort(const PipelineGraph& graph,
                                          std::vector<std::string>& order) const {
    // Build adjacency list and in-degree count.
    auto node_ids = graph.list_nodes();
    std::unordered_map<std::string, std::vector<std::string>> adj;
    std::unordered_map<std::string, std::size_t> in_degree;

    for (const auto& id : node_ids) {
        adj[id] = {};
        in_degree[id] = 0;
    }

    auto edge_ids = graph.list_edges();
    for (const auto& eid : edge_ids) {
        auto* edge = graph.find_edge(eid);
        if (edge) {
            adj[edge->source_uuid()].push_back(edge->dest_uuid());
            in_degree[edge->dest_uuid()]++;
        }
    }

    // Start with nodes that have no incoming edges.
    std::queue<std::string> queue;
    for (const auto& [id, deg] : in_degree) {
        if (deg == 0) queue.push(id);
    }

    order.clear();
    while (!queue.empty()) {
        auto current = queue.front();
        queue.pop();
        order.push_back(current);

        for (const auto& neighbor : adj[current]) {
            in_degree[neighbor]--;
            if (in_degree[neighbor] == 0) {
                queue.push(neighbor);
            }
        }
    }

    // If not all nodes were visited, there's a cycle.
    return order.size() == node_ids.size();
}

// ── Event helper ────────────────────────────────────────────────────────

void PipelineExecutor::publish_event(EventType type, const std::string& source, const std::string& message) {
    if (event_bus_) {
        Event event(type, source, message);
        event_bus_->publish(event);
    }
}

} // namespace liz
