#pragma once

#include "engine/services/ServiceType.h"

#include <string>

namespace liz {

/// Forward declaration — avoids heavy include in the header.
class ServiceRegistry;
class Service;

/// Simple locator interface for accessing registered services.
///
/// This sprint introduces ServiceLocator as a thin convenience layer
/// over the ServiceRegistry.  It does NOT replace direct Engine access
/// yet — it prepares the transition path for future sprints.
///
/// Usage:
///   ServiceLocator::init(registry);
///   auto* svc = ServiceLocator::get("Logger");
///
///   // Type-based shorthand:
///   auto* logger = ServiceLocator::get_by_type(ServiceType::Logger);
class ServiceLocator {
public:
    // Non-instantiable — all methods are static.
    ServiceLocator() = delete;

    // ── Initialization ──────────────────────────────────────────────────────

    /// Bind the locator to a ServiceRegistry instance.
    /// Pass nullptr to unbind.
    static void init(ServiceRegistry* registry);

    /// Check if a registry has been bound.
    static bool is_bound();

    // ── Lookup by name ─────────────────────────────────────────────────────

    /// Get a service by name.  Returns nullptr if not found or not bound.
    static Service* get(const std::string& name);

    // ── Lookup by type ────────────────────────────────────────────────────

    /// Get the first service matching a given ServiceType.
    /// Returns nullptr if not found or not bound.
    static Service* get_by_type(ServiceType type);

    // ── Unbind ───────────────────────────────────────────────────────────

    /// Remove the registry binding.
    static void reset();
};

} // namespace liz
