// LIZ Vision — Desktop Application Entry Point
// Sprint 1-16: Engine + all module demos.

#include "engine/core/Engine.h"
#include "engine/core/Logger.h"
#include "demo/DemoRunner.h"
#include "demo/LegacyDemo.h"
#include "demo/Sprint14Demo.h"
#include "demo/Sprint15Demo.h"
#include "demo/Sprint16Demo.h"

#include <iostream>
#include <memory>

int main() {
    std::cout << std::endl;
    std::cout << "LIZ Vision v0.1.0 — AI Video Enhancement Engine" << std::endl;
    std::cout << std::endl;

    // -- Engine initialization --
    liz::Engine engine;
    if (!engine.initialize()) {
        std::cerr << "Failed to initialize LIZ Vision Engine" << std::endl;
        return 1;
    }

    // -- Register all demos dynamically --
    DemoRegistry registry;
    registry.register_demo(std::make_unique<LegacyDemo>());
    registry.register_demo(std::make_unique<Sprint14Demo>());
    registry.register_demo(std::make_unique<Sprint15Demo>());
    registry.register_demo(std::make_unique<Sprint16Demo>());

    // -- List registered demos --
    {
        auto names = registry.list();
        std::cout << "Registered demos (" << registry.count() << "):" << std::endl;
        for (const auto& n : names) {
            std::cout << "  - " << n << std::endl;
        }
        std::cout << std::endl;
    }

    // -- Run all demos --
    int result = registry.run_all();
    if (result != 0) {
        std::cerr << "One or more demos failed" << std::endl;
        engine.shutdown();
        return 1;
    }

    // -- Cleanup --
    registry.clear();
    engine.shutdown();

    std::cout << std::endl;
    return 0;
}
