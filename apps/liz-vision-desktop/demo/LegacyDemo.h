#pragma once

#include "demo/DemoRunner.h"

/// Legacy demo covering Sprints 1-13.
/// Runs the full pipeline demonstration from previous sprints.
class LegacyDemo : public DemoRunner {
public:
    std::string name() const override { return "Legacy (Sprints 1-13)"; }
    int run() override;
};
