#pragma once

#include "demo/DemoRunner.h"

/// Sprint 18 demo: Workspace Foundation.
/// Demonstrates creating a workspace, adding projects, assets, pipelines,
/// modifying preferences/layout, and showing statistics/events/diagnostics.
class Sprint18Demo : public DemoRunner {
public:
    std::string name() const override { return "Sprint 18 (Workspace System)"; }
    int run() override;
};
