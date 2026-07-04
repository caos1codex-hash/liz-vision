#include "demo/Sprint19Demo.h"

#include "engine/core/Logger.h"
#include "engine/events/EventBus.h"
#include "engine/events/Event.h"
#include "engine/events/EventListener.h"
#include "engine/events/EventType.h"
#include "engine/services/ServiceRegistry.h"
#include "engine/services/ServiceType.h"
#include "engine/diagnostics/DiagnosticsManager.h"
#include "engine/diagnostics/EngineStatistics.h"
#include "engine/cloud/CloudSyncManager.h"
#include "engine/cloud/CloudSyncItem.h"
#include "engine/cloud/CloudTypes.h"

#include <iostream>
#include <memory>
#include <sstream>

// -- Cloud Event Listener ----------------------------------------------------
class CloudEventListener : public liz::EventListener {
public:
    std::string_view name() const override { return "CloudEventListener"; }

    void on_event(const liz::Event& event) override {
        auto t = event.type();
        if (t == liz::EventType::CloudSyncStarted ||
            t == liz::EventType::CloudSyncCompleted ||
            t == liz::EventType::CloudSyncFailed ||
            t == liz::EventType::CloudConflictDetected) {
            std::ostringstream oss;
            oss << "  [CloudEventListener] " << event.info();
            LIZ_INFO(oss.str());
        }
    }

private:
    using liz::EventListener::on_event;
};

// -- Demo implementation -------------------------------------------------------
int Sprint19Demo::run() {
    LIZ_INFO("--- Cloud Sync Foundation (Sprint 19) ---");

    liz::EventBus bus;
    CloudEventListener listener;
    bus.subscribe(&listener);

    liz::ServiceRegistry registry;
    registry.register_service("CloudSyncManager", liz::ServiceType::CloudSyncManager, "1.0.0");

    std::cout << std::endl;

    // ==================================================================
    // 1. Create CloudSyncManager
    // ==================================================================
    LIZ_INFO("[1] Creating CloudSyncManager...");

    liz::CloudSyncManager cloud_mgr;
    cloud_mgr.initialize();
    cloud_mgr.set_event_bus(&bus);

    LIZ_INFO("  CloudSyncManager initialized and connected to EventBus");

    std::cout << std::endl;

    // ==================================================================
    // 2. Create 5 sync items (one of each type)
    // ==================================================================
    LIZ_INFO("[2] Creating 5 cloud sync items...");

    liz::CloudSyncItem item_project("Proyecto Video", liz::CloudSyncType::Project);
    liz::CloudSyncItem item_workspace("LIZ Studio", liz::CloudSyncType::Workspace);
    liz::CloudSyncItem item_asset("video_input.mp4", liz::CloudSyncType::Asset);
    liz::CloudSyncItem item_pipeline("Studio Pipeline", liz::CloudSyncType::Pipeline);
    liz::CloudSyncItem item_settings("App Preferences", liz::CloudSyncType::Settings);

    {
        std::ostringstream oss;
        oss << "  " << item_project.to_string();
        LIZ_INFO(oss.str());
    }
    {
        std::ostringstream oss;
        oss << "  " << item_workspace.to_string();
        LIZ_INFO(oss.str());
    }
    {
        std::ostringstream oss;
        oss << "  " << item_asset.to_string();
        LIZ_INFO(oss.str());
    }
    {
        std::ostringstream oss;
        oss << "  " << item_pipeline.to_string();
        LIZ_INFO(oss.str());
    }
    {
        std::ostringstream oss;
        oss << "  " << item_settings.to_string();
        LIZ_INFO(oss.str());
    }

    std::cout << std::endl;

    // ==================================================================
    // 3. Enqueue all items
    // ==================================================================
    LIZ_INFO("[3] Enqueuing items into cloud sync queue...");

    cloud_mgr.enqueue(std::move(item_project));
    cloud_mgr.enqueue(std::move(item_workspace));
    cloud_mgr.enqueue(std::move(item_asset));
    cloud_mgr.enqueue(std::move(item_pipeline));
    cloud_mgr.enqueue(std::move(item_settings));

    {
        std::ostringstream oss;
        oss << "  Pending items in queue: " << cloud_mgr.pending_count();
        LIZ_INFO(oss.str());
    }

    std::cout << std::endl;

    // ==================================================================
    // 4. Process all items (simulated cloud sync: 70% Synced / 30% Conflict)
    // ==================================================================
    LIZ_INFO("[4] Processing all items (70% Synced / 30% Conflict simulation)...");

    cloud_mgr.process_all();

    {
        std::ostringstream oss;
        oss << "  Processed items: " << cloud_mgr.processed_items().size();
        LIZ_INFO(oss.str());
    }

    std::cout << std::endl;

    // ==================================================================
    // 5. Show final states of processed items
    // ==================================================================
    LIZ_INFO("[5] Final states of processed items:");

    for (const auto& item : cloud_mgr.processed_items()) {
        std::ostringstream oss;
        oss << "  " << item.to_string();
        LIZ_INFO(oss.str());
    }

    std::cout << std::endl;

    // ==================================================================
    // 6. Show events from EventBus
    // ==================================================================
    LIZ_INFO("[6] EventBus statistics:");
    {
        auto bus_stats = bus.statistics();
        std::ostringstream oss;
        oss << "  Total published: " << bus_stats.total_published
            << "  Total delivered: " << bus_stats.total_delivered
            << "  Listeners: " << bus_stats.listener_count;
        LIZ_INFO(oss.str());
    }

    std::cout << std::endl;

    // ==================================================================
    // 7. Show cloud sync statistics
    // ==================================================================
    LIZ_INFO("[7] Cloud sync statistics:");
    {
        std::ostringstream oss;
        oss << "  Synced:    " << cloud_mgr.synced_count();
        LIZ_INFO(oss.str());
    }
    {
        std::ostringstream oss;
        oss << "  Conflicts: " << cloud_mgr.conflict_count();
        LIZ_INFO(oss.str());
    }
    {
        std::ostringstream oss;
        oss << "  Pending:   " << cloud_mgr.pending_count();
        LIZ_INFO(oss.str());
    }
    {
        std::ostringstream oss;
        oss << "  Sync time: " << std::fixed << cloud_mgr.total_sync_time_ms() << " ms";
        LIZ_INFO(oss.str());
    }

    std::cout << std::endl;

    // ==================================================================
    // 8. Diagnostics report with cloud data
    // ==================================================================
    LIZ_INFO("[8] Diagnostics report (updated with cloud metrics):");
    {
        liz::DiagnosticsDataProvider provider;
        provider.events_published   = bus.event_count();
        provider.services_registered = registry.count();

        // Cloud metrics
        provider.cloud_pending     = cloud_mgr.pending_count();
        provider.cloud_synced      = cloud_mgr.synced_count();
        provider.cloud_conflicts   = cloud_mgr.conflict_count();
        provider.cloud_sync_time_ms = cloud_mgr.total_sync_time_ms();

        liz::DiagnosticsManager diag;
        diag.set_provider(&provider);
        diag.capture_snapshot("after_sprint19");

        auto engine_stats = diag.statistics();
        std::ostringstream oss;
        oss << "  " << engine_stats.to_string();
        LIZ_INFO(oss.str());

        diag.print_report();
    }

    std::cout << std::endl;

    // ==================================================================
    // Cleanup
    // ==================================================================
    cloud_mgr.clear();
    cloud_mgr.shutdown();
    registry.clear();
    bus.unsubscribe(&listener);

    LIZ_INFO("Cloud Sync demo completed");
    return 0;
}
