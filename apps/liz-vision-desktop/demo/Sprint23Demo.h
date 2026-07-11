#pragma once

#include "demo/DemoRunner.h"

/// Sprint 23 demo: Advanced Configuration System.
/// Demonstrates schema validation, runtime overrides, execution profiles,
/// dynamic reload, and integration with EventBus/ServiceRegistry/Diagnostics
/// and the public EngineAPI — building on top of the Sprint 22 Foundation.
class Sprint23Demo : public DemoRunner {
public:
    std::string name() const override { return "Sprint 23 (Advanced Configuration System)"; }
    int run() override;
};
