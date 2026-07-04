#pragma once

#include "demo/DemoRunner.h"

/// Sprint 19 demo: Cloud Sync Foundation.
/// Demonstrates cloud sync queue with 5 items (Project, Workspace, Asset,
/// Pipeline, Settings), simulated 70/30 sync/conflict, EventBus events,
/// and statistics.
class Sprint19Demo : public DemoRunner {
public:
    std::string name() const override { return "Sprint 19 (Cloud Sync)"; }
    int run() override;
};
