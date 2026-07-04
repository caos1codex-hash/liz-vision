#pragma once

#include "demo/DemoRunner.h"

/// Sprint 16 demo: Pipeline Graph Foundation.
/// Demonstrates creating a DAG pipeline, adding nodes, connecting them,
/// validating, executing, and showing statistics/events/logs.
class Sprint16Demo : public DemoRunner {
public:
    std::string name() const override { return "Sprint 16 (Pipeline Graph)"; }
    int run() override;
};
