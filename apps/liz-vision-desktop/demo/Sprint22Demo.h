#pragma once

#include "demo/DemoRunner.h"

/// Sprint 22 demo: Configuration System Foundation.
/// Demonstrates in-memory configuration with sections, values,
/// EventBus events, Diagnostics integration, and EngineAPI exposure.
class Sprint22Demo : public DemoRunner {
public:
    std::string name() const override { return "Sprint 22 (Configuration System)"; }
    int run() override;
};
