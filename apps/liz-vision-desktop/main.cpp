// LIZ Vision — Desktop Application Entry Point
// Sprint 1-14: Engine + all module demos.

#include "engine/core/Engine.h"
#include "engine/core/Logger.h"
#include "demo/DemoRunner.h"
#include "demo/LegacyDemo.h"
#include "demo/Sprint14Demo.h"

#include <iostream>

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

    // -- Run Legacy Demo (Sprints 1-13) --
    {
        LegacyDemo legacy;
        if (legacy.run() != 0) {
            std::cerr << "Legacy demo failed" << std::endl;
            return 1;
        }
    }

    // -- Run Sprint 14 Demo (Asset System Foundation) --
    {
        Sprint14Demo sprint14;
        if (sprint14.run() != 0) {
            std::cerr << "Sprint 14 demo failed" << std::endl;
            return 1;
        }
    }

    // -- Cleanup --
    engine.shutdown();

    std::cout << std::endl;
    return 0;
}
