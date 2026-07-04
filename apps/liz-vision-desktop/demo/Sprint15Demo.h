#pragma once

#include "demo/DemoRunner.h"

/// Sprint 15 demo: Public API Foundation.
/// Demonstrates the EngineBuilder -> EngineAPI -> EngineSession workflow.
/// Uses EXCLUSIVELY the public API — no internal Engine access.
class Sprint15Demo : public DemoRunner {
public:
    std::string name() const override { return "Sprint 15 (Public API)"; }
    int run() override;
};
