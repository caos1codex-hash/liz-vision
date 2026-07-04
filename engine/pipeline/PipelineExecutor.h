#pragma once

#include "engine/pipeline/PipelineGraph.h"
#include "engine/pipeline/PipelineStatistics.h"
#include "engine/events/EventType.h"

#include <chrono>
#include <memory>
#include <string>
#include <vector>

namespace liz {

class EventBus;

/// Executes a pipeline graph by traversing its nodes in topological order.
///
/// Execution is completely simulated in this sprint.
/// The executor:
///   1. Validates the graph (no cycles)
///   2. Computes topological execution order
///   3. Executes each node, changing states
///   4. Updates statistics
///   5. Publishes events
class PipelineExecutor {
public:
    PipelineExecutor();
    ~PipelineExecutor() = default;

    // Non-copyable.
    PipelineExecutor(const PipelineExecutor&) = delete;
    PipelineExecutor& operator=(const PipelineExecutor&) = delete;

    // ── Lifecycle ─────────────────────────────────────────────────────

    /// Initialize the executor.
    bool initialize();

    /// Shut down the executor.
    void shutdown();

    // ── Pipeline management ───────────────────────────────────────────

    /// Register a pipeline for execution.
    bool register_pipeline(std::unique_ptr<PipelineGraph> graph);

    /// Destroy a pipeline by name.
    bool destroy_pipeline(const std::string& name);

    /// Get a pipeline by name.
    PipelineGraph* find_pipeline(const std::string& name) const;

    /// List all registered pipeline names.
    std::vector<std::string> list_pipelines() const;

    /// Number of registered pipelines.
    std::size_t pipeline_count() const;

    // ── Execution ────────────────────────────────────────────────────

    /// Execute a pipeline by name.  Returns statistics.
    PipelineStatistics execute(const std::string& name);

    /// Execute all registered pipelines.
    void execute_all();

    // ── Statistics ────────────────────────────────────────────────────

    /// Get statistics for a specific pipeline.
    PipelineStatistics pipeline_statistics(const std::string& name) const;

    /// Get global statistics across all pipelines.
    PipelineStatistics global_statistics() const;

    // ── EventBus ──────────────────────────────────────────────────────

    void set_event_bus(EventBus* bus);

    // ── Bulk ──────────────────────────────────────────────────────────

    /// Clear all registered pipelines.
    void clear();

private:
    // ── Topological sort ──────────────────────────────────────────────
    /// Compute the execution order using Kahn's algorithm.
    /// Returns false if a cycle is detected.
    bool topological_sort(const PipelineGraph& graph,
                          std::vector<std::string>& order) const;

    void publish_event(EventType type, const std::string& source, const std::string& message);

    struct Entry {
        std::unique_ptr<PipelineGraph> graph;
    };

    std::vector<Entry> entries_;
    EventBus*         event_bus_   = nullptr;
    bool              initialized_ = false;

    // Global counters.
    std::size_t total_executions_ = 0;
    std::size_t total_failures_   = 0;
};

} // namespace liz
