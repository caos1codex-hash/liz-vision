#pragma once

#include "demo/DemoRunner.h"

/// Sprint 21 demo: Job System Foundation.
/// Demonstrates job submission, queueing, Scheduler integration,
/// EventBus events, Diagnostics integration, and EngineAPI exposure.
class Sprint21Demo : public DemoRunner {
public:
    std::string name() const override { return "Sprint 21 (Job System)"; }
    int run() override;
};
