#pragma once

#include "demo/DemoRunner.h"

/// Sprint 17 demo: Project System Foundation.
/// Demonstrates creating a project, modifying metadata and settings,
/// simulating assets and pipelines, saving, closing, and showing
/// statistics, events, and diagnostics.
class Sprint17Demo : public DemoRunner {
public:
    std::string name() const override { return "Sprint 17 (Project System)"; }
    int run() override;
};
