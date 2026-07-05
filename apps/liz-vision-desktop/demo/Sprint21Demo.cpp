#include "demo/Sprint21Demo.h"

#include "engine/core/Logger.h"
#include "engine/core/Engine.h"
#include "engine/events/EventBus.h"
#include "engine/events/Event.h"
#include "engine/events/EventListener.h"
#include "engine/events/EventType.h"
#include "engine/services/ServiceRegistry.h"
#include "engine/services/ServiceType.h"
#include "engine/diagnostics/DiagnosticsManager.h"
#include "engine/diagnostics/EngineStatistics.h"
#include "engine/jobs/JobManager.h"
#include "engine/jobs/Job.h"
#include "engine/jobs/JobTypes.h"
#include "engine/jobs/JobQueue.h"
#include "engine/jobs/JobStatistics.h"
#include "engine/api/EngineAPI.h"
#include "engine/api/ApiTypes.h"
#include "engine/api/EngineBuilder.h"

#include <iostream>
#include <memory>
#include <sstream>
#include <vector>

// -- Job Event Listener ---------------------------------------------------
class JobEventListener : public liz::EventListener {
public:
    std::string_view name() const override { return "JobEventListener"; }

    void on_event(const liz::Event& event) override {
        auto t = event.type();
        if (t == liz::EventType::JobCreated ||
            t == liz::EventType::JobQueued ||
            t == liz::EventType::JobStarted ||
            t == liz::EventType::JobCompleted ||
            t == liz::EventType::JobFailed ||
            t == liz::EventType::JobCancelled) {
            std::ostringstream oss;
            oss << "  [JobEventListener] " << event.info();
            LIZ_INFO(oss.str());
        }
    }

private:
    using liz::EventListener::on_event;
};

// -- Demo implementation ─────────────────────────────────────────────────
int Sprint21Demo::run() {
    LIZ_INFO("--- Job System Foundation (Sprint 21) ---");

    liz::EventBus bus;
    JobEventListener listener;
    bus.subscribe(&listener);

    liz::ServiceRegistry registry;
    registry.register_service("JobManager", liz::ServiceType::JobManager, "1.0.0");

    // Create an Engine to get access to its Scheduler.
    liz::Engine engine;
    engine.initialize();

    std::cout << std::endl;

    // ==================================================================
    // 1. Create JobManager and connect to Scheduler + EventBus
    // ==================================================================
    LIZ_INFO("[1] Creating JobManager...");

    liz::JobManager job_manager;
    job_manager.initialize(&engine.scheduler());
    job_manager.set_event_bus(&bus);

    LIZ_INFO("  JobManager initialized and connected to Scheduler + EventBus");

    std::cout << std::endl;

    // ==================================================================
    // 2. Create 10 jobs with different types and priorities
    // ==================================================================
    LIZ_INFO("[2] Creating and submitting 10 jobs...");

    struct JobSpec {
        std::string name;
        liz::JobType type;
        liz::JobPriority priority;
    };

    std::vector<JobSpec> specs = {
        {"Render Frame",       liz::JobType::Render,     liz::JobPriority::High},
        {"AI Upscaling",       liz::JobType::Inference,  liz::JobPriority::Critical},
        {"AI Interpolation",   liz::JobType::Inference,  liz::JobPriority::High},
        {"Import Video",       liz::JobType::Import,     liz::JobPriority::Normal},
        {"Export Video",       liz::JobType::Export,     liz::JobPriority::Normal},
        {"Cloud Upload",       liz::JobType::Cloud,      liz::JobPriority::Low},
        {"Asset Scan",         liz::JobType::Asset,      liz::JobPriority::Low},
        {"Pipeline Execute",   liz::JobType::Pipeline,   liz::JobPriority::High},
        {"Plugin Reload",      liz::JobType::Plugin,     liz::JobPriority::Normal},
        {"Workspace Backup",   liz::JobType::Workspace,  liz::JobPriority::Low}
    };

    std::vector<std::string> uuids;
    for (const auto& spec : specs) {
        auto job = std::make_unique<liz::Job>(spec.name, spec.type, spec.priority);
        std::string uuid = job_manager.submit(std::move(job));
        uuids.push_back(uuid);
    }

    {
        std::ostringstream oss;
        oss << "  Submitted: " << uuids.size() << " jobs";
        LIZ_INFO(oss.str());
    }

    for (std::size_t i = 0; i < specs.size(); ++i) {
        std::ostringstream oss;
        oss << "    [" << (i + 1) << "] " << specs[i].name
            << " (" << job_type_to_string(specs[i].type)
            << ", " << job_priority_to_string(specs[i].priority) << ")";
        LIZ_INFO(oss.str());
    }

    std::cout << std::endl;

    // ==================================================================
    // 3. Execute all jobs (JobManager converts to Scheduler Tasks)
    // ==================================================================
    LIZ_INFO("[3] Processing all jobs (via Scheduler integration)...");

    std::size_t processed = job_manager.process_all();

    {
        std::ostringstream oss;
        oss << "  Processed: " << processed << " jobs";
        LIZ_INFO(oss.str());
    }

    std::cout << std::endl;

    // ==================================================================
    // 4. Show job results — state, progress, duration
    // ==================================================================
    LIZ_INFO("[4] Job results:");

    auto completed = job_manager.completed_jobs();
    for (const auto* job : completed) {
        std::ostringstream oss;
        oss << "  [Completed] " << job->to_string();
        LIZ_INFO(oss.str());
    }

    auto failed = job_manager.failed_jobs();
    for (const auto* job : failed) {
        std::ostringstream oss;
        oss << "  [Failed] " << job->to_string();
        LIZ_INFO(oss.str());
    }

    std::cout << std::endl;

    // ==================================================================
    // 5. Show JobManager statistics
    // ==================================================================
    LIZ_INFO("[5] JobManager statistics:");
    {
        auto stats = job_manager.statistics();
        std::ostringstream oss;
        oss << "  " << stats.to_string();
        LIZ_INFO(oss.str());
    }

    std::cout << std::endl;

    // ==================================================================
    // 6. Show EventBus statistics
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
    // 7. Diagnostics report with job data
    // ==================================================================
    LIZ_INFO("[7] Diagnostics report (updated with job metrics):");
    {
        liz::DiagnosticsDataProvider provider;
        provider.events_published   = bus.event_count();
        provider.services_registered = registry.count();

        auto js = job_manager.statistics();
        provider.jobs_created          = js.jobs_created;
        provider.jobs_running          = js.jobs_running;
        provider.jobs_completed        = js.jobs_completed;
        provider.jobs_failed           = js.jobs_failed;
        provider.job_execution_time_ms = js.job_execution_time_ms;

        liz::DiagnosticsManager diag;
        diag.set_provider(&provider);
        diag.capture_snapshot("after_sprint21");

        auto engine_stats = diag.statistics();
        std::ostringstream oss;
        oss << "  " << engine_stats.to_string();
        LIZ_INFO(oss.str());

        diag.print_report();
    }

    std::cout << std::endl;

    // ==================================================================
    // 8. Demonstrate EngineAPI integration
    // ==================================================================
    LIZ_INFO("[8] EngineAPI integration test:");
    {
        liz::EngineBuilder builder;
        builder.set_application_name("Sprint21Test")
               .set_application_version("1.0.0")
               .enable_gpu(false)
               .enable_diagnostics(false)
               .enable_events(true)
               .build();

        liz::EngineAPI api;
        auto result = api.initialize(builder);

        if (result == liz::ApiResult::Success) {
            // Submit a job via API.
            liz::JobInfo job_info;
            api.submit_job("API Test Job", "Render", job_info);
            {
                std::ostringstream oss;
                oss << "  API submitted job: " << job_info.name
                    << " uuid=" << job_info.uuid
                    << " state=" << job_info.state;
                LIZ_INFO(oss.str());
            }

            // Get job statistics.
            auto api_stats = api.job_statistics();
            {
                std::ostringstream oss;
                oss << "  API job statistics: " << api_stats.to_string();
                LIZ_INFO(oss.str());
            }

            // List jobs.
            auto jobs = api.list_jobs();
            {
                std::ostringstream oss;
                oss << "  API jobs listed: " << jobs.size();
                LIZ_INFO(oss.str());
            }

            api.shutdown();
        } else {
            LIZ_WARN("  EngineAPI initialization failed");
        }
    }

    std::cout << std::endl;

    // ==================================================================
    // Cleanup
    // ==================================================================
    job_manager.clear();
    job_manager.shutdown();
    registry.clear();
    bus.unsubscribe(&listener);
    engine.shutdown();

    LIZ_INFO("Job System demo completed");
    return 0;
}
