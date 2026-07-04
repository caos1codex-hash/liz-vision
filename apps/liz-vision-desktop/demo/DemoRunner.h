#pragma once

#include <memory>
#include <string>
#include <vector>
#include <functional>

/// Interface for sprint demo runners.
/// Each sprint implements this to keep demo logic out of main().
class DemoRunner {
public:
    virtual ~DemoRunner() = default;

    /// Human-readable name for this demo.
    virtual std::string name() const = 0;

    /// Execute the demo.  Return 0 on success.
    virtual int run() = 0;
};

/// Registry for dynamically managing demos.
/// Uses polymorphism — no giant if-else chains.
class DemoRegistry {
public:
    DemoRegistry() = default;
    ~DemoRegistry() = default;

    // Non-copyable.
    DemoRegistry(const DemoRegistry&) = delete;
    DemoRegistry& operator=(const DemoRegistry&) = delete;

    /// Register a demo by unique name.  Takes ownership.
    /// Returns false if a demo with the same name already exists.
    bool register_demo(std::unique_ptr<DemoRunner> demo);

    /// Remove a demo by name.
    bool remove_demo(const std::string& name);

    /// Remove all registered demos.
    void clear();

    /// Run a specific demo by name.  Returns 0 on success, -1 if not found.
    int run_demo(const std::string& name);

    /// Run all registered demos in insertion order.
    /// Stops on first failure.  Returns 0 if all passed.
    int run_all();

    /// List all registered demo names.
    std::vector<std::string> list() const;

    /// Number of registered demos.
    std::size_t count() const;

    /// Check if a demo with the given name is registered.
    bool has(const std::string& name) const;

private:
    struct Entry {
        std::unique_ptr<DemoRunner> demo;
    };

    std::vector<Entry> entries_;
};
