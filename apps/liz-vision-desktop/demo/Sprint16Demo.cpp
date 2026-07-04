#include "demo/Sprint16Demo.h"

#include "engine/core/Logger.h"
#include "engine/events/EventBus.h"
#include "engine/events/Event.h"
#include "engine/events/EventListener.h"
#include "engine/events/EventType.h"
#include "engine/services/ServiceRegistry.h"
#include "engine/services/ServiceType.h"
#include "engine/diagnostics/DiagnosticsManager.h"
#include "engine/diagnostics/PerformanceSnapshot.h"
#include "engine/diagnostics/EngineStatistics.h"
#include "engine/pipeline/PipelineGraph.h"
#include "engine/pipeline/PipelineNode.h"
#include "engine/pipeline/PipelineEdge.h"
#include "engine/pipeline/PipelineExecutor.h"
#include "engine/pipeline/PipelineStatistics.h"
#include "engine/pipeline/PipelineTypes.h"

#include <iostream>
#include <memory>
#include <sstream>

// -- Pipeline Event Listener --------------------------------------------------
class PipelineEventListener : public liz::EventListener {
public:
    std::string_view name() const override { return "PipelineEventListener"; }

    void on_event(const liz::Event& event) override {
        auto t = event.type();
        if (t == liz::EventType::PipelineCreated ||
            t == liz::EventType::PipelineDestroyed ||
            t == liz::EventType::NodeCreated ||
            t == liz::EventType::NodeExecuted ||
            t == liz::EventType::PipelineFinished) {
            std::ostringstream oss;
            oss << "  [PipelineEventListener] " << event.info();
            LIZ_INFO(oss.str());
        }
    }

private:
    using liz::EventListener::on_event;
};

// -- Demo implementation -------------------------------------------------------

int Sprint16Demo::run() {
    LIZ_INFO("--- Pipeline Graph Foundation (Sprint 16) ---");

    liz::EventBus bus;
    PipelineEventListener listener;
    bus.subscribe(&listener);

    std::cout << std::endl;

    // ═════════════════════════════════════════════════════════════════════
    // 1. Create PipelineGraph
    // ═════════════════════════════════════════════════════════════════════
    LIZ_INFO("[1] Creating PipelineGraph...");
    liz::PipelineGraph graph("Video Upscale Pipeline");
    graph.set_event_bus(&bus);

    std::cout << std::endl;

    // ═════════════════════════════════════════════════════════════════════
    // 2. Create nodes
    // ═════════════════════════════════════════════════════════════════════
    LIZ_INFO("[2] Creating pipeline nodes...");

    auto* input   = graph.create_node("Video Input",     liz::PipelineNodeType::Input);
    auto* decoder = graph.create_node("Video Decoder",   liz::PipelineNodeType::Decoder);
    auto* filter  = graph.create_node("Bicubic Filter",  liz::PipelineNodeType::VideoFilter);
    auto* tensor  = graph.create_node("Tensor Convert",  liz::PipelineNodeType::TensorConverter);
    auto* infer   = graph.create_node("LIZ Upscaler",   liz::PipelineNodeType::Inference);
    auto* output  = graph.create_node("H.264 Output",    liz::PipelineNodeType::Output);

    // Set logical positions (for visualization).
    input->set_position(0, 0);
    decoder->set_position(1, 0);
    filter->set_position(2, 0);
    tensor->set_position(3, 0);
    infer->set_position(4, 0);
    output->set_position(5, 0);

    std::cout << std::endl;

    // ═════════════════════════════════════════════════════════════════════
    // 3. Connect nodes (Input -> Decoder -> Filter -> Tensor -> Inference -> Output)
    // ═════════════════════════════════════════════════════════════════════
    LIZ_INFO("[3] Connecting pipeline nodes...");

    graph.connect(input->uuid(), decoder->uuid(), liz::ConnectionType::Data);
    graph.connect(decoder->uuid(), filter->uuid(), liz::ConnectionType::Data);
    graph.connect(filter->uuid(), tensor->uuid(), liz::ConnectionType::Data);
    graph.connect(tensor->uuid(), infer->uuid(), liz::ConnectionType::Data);
    graph.connect(infer->uuid(), output->uuid(), liz::ConnectionType::Data);

    std::cout << std::endl;

    // ═════════════════════════════════════════════════════════════════════
    // 4. List nodes and edges
    // ═════════════════════════════════════════════════════════════════════
    LIZ_INFO("[4] Graph summary:");
    {
        std::ostringstream oss;
        oss << "  Nodes: " << graph.node_count() << "  Edges: " << graph.edge_count();
        LIZ_INFO(oss.str());
    }

    auto node_ids = graph.list_nodes();
    for (const auto& id : node_ids) {
        auto* node = graph.find_node(id);
        if (node) {
            std::ostringstream oss;
            oss << "  " << node->info();
            LIZ_INFO(oss.str());
        }
    }

    std::cout << std::endl;

    // ═════════════════════════════════════════════════════════════════════
    // 5. Validate the graph
    // ═════════════════════════════════════════════════════════════════════
    LIZ_INFO("[5] Validating pipeline graph...");
    bool valid = graph.validate();
    {
        std::ostringstream oss;
        oss << "  Validation result: " << (valid ? "PASS (valid DAG)" : "FAIL (cycle detected)");
        LIZ_INFO(oss.str());
    }

    std::cout << std::endl;

    // ═════════════════════════════════════════════════════════════════════
    // 6. Test cycle prevention
    // ═════════════════════════════════════════════════════════════════════
    LIZ_INFO("[6] Testing cycle prevention...");
    bool cycle_rejected = !graph.connect(output->uuid(), input->uuid(), liz::ConnectionType::Data);
    {
        std::ostringstream oss;
        oss << "  Output -> Input connection rejected (cycle): " << (cycle_rejected ? "true" : "false");
        LIZ_INFO(oss.str());
    }

    std::cout << std::endl;

    // ═════════════════════════════════════════════════════════════════════
    // 7. Execute pipeline via PipelineExecutor
    // ═════════════════════════════════════════════════════════════════════
    LIZ_INFO("[7] Executing pipeline...");

    liz::PipelineExecutor executor;
    executor.initialize();
    executor.set_event_bus(&bus);

    // Move the graph into the executor.
    auto extracted_graph = std::make_unique<liz::PipelineGraph>(std::move(graph));
    executor.register_pipeline(std::move(extracted_graph));

    auto stats = executor.execute("Video Upscale Pipeline");

    std::cout << std::endl;

    // ═════════════════════════════════════════════════════════════════════
    // 8. Execution statistics
    // ═════════════════════════════════════════════════════════════════════
    LIZ_INFO("[8] Execution statistics:");
    {
        std::ostringstream oss;
        oss << "  " << stats.to_string();
        LIZ_INFO(oss.str());
    }

    std::cout << std::endl;

    // ═════════════════════════════════════════════════════════════════════
    // 9. Service Registry integration
    // ═════════════════════════════════════════════════════════════════════
    LIZ_INFO("[9] Service Registry integration:");
    {
        liz::ServiceRegistry registry;
        registry.register_service("PipelineExecutor", liz::ServiceType::PipelineExecutor, "1.0.0");
        registry.log_statistics();
        registry.clear();
    }

    std::cout << std::endl;

    // ═════════════════════════════════════════════════════════════════════
    // 10. Diagnostics integration
    // ═════════════════════════════════════════════════════════════════════
    LIZ_INFO("[10] Diagnostics integration:");
    {
        liz::DiagnosticsDataProvider provider;
        provider.pipelines_active     = executor.pipeline_count();
        provider.pipeline_nodes       = stats.total_nodes;
        provider.pipeline_exec_time_ms = stats.execution_time_ms;
        provider.pipeline_errors      = stats.nodes_failed;

        liz::DiagnosticsManager diag;
        diag.set_provider(&provider);

        auto engine_stats = diag.statistics();
        std::ostringstream oss;
        oss << "  " << engine_stats.to_string();
        LIZ_INFO(oss.str());
    }

    std::cout << std::endl;

    // ═════════════════════════════════════════════════════════════════════
    // 11. Cleanup
    // ═════════════════════════════════════════════════════════════════════
    executor.clear();
    executor.shutdown();
    bus.unsubscribe(&listener);

    LIZ_INFO("Pipeline Graph demo completed");
    return 0;
}
