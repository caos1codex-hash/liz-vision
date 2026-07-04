#pragma once

#include "engine/services/Service.h"

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

namespace liz {

/// Statistics snapshot of the ServiceRegistry.
struct ServiceRegistryStats {
    std::size_t total_registered = 0;
    std::size_t total_removed    = 0;
    std::size_t duplicate_attempts = 0;
};

/// Central registry for all engine services.
///
/// Services are registered by name.  Each name must be unique —
/// attempting to register a duplicate logs a warning and increments
/// the duplicate counter (does NOT overwrite).
///
/// Usage:
///   ServiceRegistry registry;
///   registry.register_service("Logger", ServiceType::Logger, "1.0.0");
///   auto* svc = registry.find("Logger");
///   registry.unregister_service("Logger");
class ServiceRegistry {
public:
    ServiceRegistry();
    ~ServiceRegistry() = default;

    // Non-copyable.
    ServiceRegistry(const ServiceRegistry&) = delete;
    ServiceRegistry& operator=(const ServiceRegistry&) = delete;

    // ── Registration ────────────────────────────────────────────────────────

    /// Register a new service by name, type, and version.
    /// Returns true on success, false if the name already exists.
    bool register_service(const std::string& name,
                          ServiceType type,
                          const std::string& version);

    /// Unregister a service by name.  Returns true if found and removed.
    bool unregister_service(const std::string& name);

    // ── Query ───────────────────────────────────────────────────────────────

    /// Find a service by name.  Returns nullptr if not found.
    Service* find(const std::string& name) const;

    /// Check if a service with the given name exists.
    bool exists(const std::string& name) const;

    /// Number of currently registered services.
    std::size_t count() const;

    /// Get the names of all registered services.
    std::vector<std::string> list() const;

    // ── Bulk operations ─────────────────────────────────────────────────────

    /// Remove all registered services.
    void clear();

    // ── Statistics ─────────────────────────────────────────────────────────

    /// Get a statistics snapshot.
    ServiceRegistryStats statistics() const;

    /// Log a summary of the registry statistics.
    void log_statistics() const;

private:
    struct Entry {
        std::unique_ptr<Service> service;
    };

    std::vector<Entry>         entries_;
    std::size_t                total_registered_    = 0;
    std::size_t                total_removed_       = 0;
    std::size_t                duplicate_attempts_  = 0;
};

} // namespace liz
