// LIZ Vision — Desktop Application Entry Point
// Sprint 1 Demo: Engine initialization, plugin registration, task execution.

#include "engine/core/Engine.h"
#include "engine/core/Logger.h"
#include "engine/plugins/PluginInterface.h"

#include <iostream>
#include <memory>
#include <sstream>

// ── Dummy Plugin ──────────────────────────────────────────────────────────────
class DummyPlugin : public liz::PluginInterface {
public:
    explicit DummyPlugin(std::string name, std::string category = "test")
        : name_(std::move(name)), category_(std::move(category)) {}

    std::string_view name() const override { return name_; }
    std::string_view version() const override { return "0.1.0"; }
    std::string_view category() const override { return category_; }

    void on_register() override {
        std::ostringstream oss;
        oss << "DummyPlugin '" << name_ << "': on_register() called";
        LIZ_INFO(oss.str());
    }

    void on_unregister() override {
        std::ostringstream oss;
        oss << "DummyPlugin '" << name_ << "': on_unregister() called";
        LIZ_INFO(oss.str());
    }

private:
    std::string name_;
    std::string category_;
};

// ── Main ──────────────────────────────────────────────────────────────────────
int main() {
    std::cout << std::endl;

    // 1. Create and initialize the Engine
    liz::Engine engine;
    if (!engine.initialize()) {
        std::cerr << "Failed to initialize LIZ Vision Engine" << std::endl;
        return 1;
    }

    std::cout << std::endl;

    // 2. Register a dummy plugin
    auto plugin = std::make_shared<DummyPlugin>("Demo Upscaler", "upscaler");
    engine.register_plugin(plugin);

    auto plugin2 = std::make_shared<DummyPlugin>("Demo Detector", "detection");
    engine.register_plugin(plugin2);

    std::cout << std::endl;

    // 3. List registered plugins
    auto names = engine.plugin_manager().list_names();
    {
        std::ostringstream oss;
        oss << "Registered plugins (" << names.size() << "): ";
        for (std::size_t i = 0; i < names.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << names[i];
        }
        LIZ_INFO(oss.str());
    }

    std::cout << std::endl;

    // 4. Submit dummy tasks
    engine.submit_task("Initialize config",
        []() {
            LIZ_INFO("  -> Config check passed");
            return true;
        });

    engine.submit_task("Load resources",
        []() {
            LIZ_INFO("  -> Resources loaded (simulated)");
            return true;
        });

    engine.submit_task("Run pipeline step 1",
        []() {
            LIZ_INFO("  -> Pipeline step 1 done");
            return true;
        });

    engine.submit_task("Run pipeline step 2",
        []() {
            LIZ_INFO("  -> Pipeline step 2 done");
            return true;
        });

    // Submit a task that intentionally fails
    engine.submit_task("Intentional failure demo",
        []() {
            LIZ_WARN("  -> This task will return false");
            return false;
        });

    std::cout << std::endl;

    // 5. Show pending count
    {
        std::ostringstream oss;
        oss << "Pending tasks: " << engine.scheduler().pending_count();
        LIZ_INFO(oss.str());
    }

    std::cout << std::endl;

    // 6. Execute all pending tasks
    auto executed = engine.run_pending();

    {
        std::ostringstream oss;
        oss << "Executed " << executed << " tasks";
        LIZ_INFO(oss.str());
    }

    std::cout << std::endl;

    // 7. Shutdown
    engine.shutdown();

    std::cout << std::endl;
    return 0;
}