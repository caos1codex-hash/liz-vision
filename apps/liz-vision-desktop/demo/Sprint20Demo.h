#pragma once

#include "demo/DemoRunner.h"

/// Sprint 20 demo: Plugin Loader Foundation.
/// Demonstrates simulated plugin registration, loading, reloading,
/// unloading, EventBus events, and Diagnostics integration.
class Sprint20Demo : public DemoRunner {
public:
    std::string name() const override { return "Sprint 20 (Plugin Loader)"; }
    int run() override;
};
