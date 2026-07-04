#include "engine/services/ServiceLocator.h"
#include "engine/services/ServiceRegistry.h"
#include "engine/core/Logger.h"

#include <sstream>

namespace liz {

// ── Internal registry pointer ──────────────────────────────────────────────

static ServiceRegistry* g_registry = nullptr;

// ── Initialization ─────────────────────────────────────────────────────────

void ServiceLocator::init(ServiceRegistry* registry) {
    g_registry = registry;
    if (registry) {
        LIZ_INFO("ServiceLocator: bound to ServiceRegistry");
    } else {
        LIZ_INFO("ServiceLocator: unbound from ServiceRegistry");
    }
}

bool ServiceLocator::is_bound() {
    return g_registry != nullptr;
}

// ── Lookup by name ─────────────────────────────────────────────────────────

Service* ServiceLocator::get(const std::string& name) {
    if (!g_registry) {
        LIZ_WARN("ServiceLocator: not bound — cannot find service");
        return nullptr;
    }
    return g_registry->find(name);
}

// ── Lookup by type ────────────────────────────────────────────────────────

Service* ServiceLocator::get_by_type(ServiceType type) {
    if (!g_registry) {
        LIZ_WARN("ServiceLocator: not bound — cannot find service by type");
        return nullptr;
    }

    // Iterate through registered services to find first match by type.
    auto names = g_registry->list();
    for (const auto& name : names) {
        auto* svc = g_registry->find(name);
        if (svc && svc->type() == type) {
            return svc;
        }
    }

    std::ostringstream oss;
    oss << "ServiceLocator: no service found for type "
        << service_type_to_string(type);
    LIZ_WARN(oss.str());
    return nullptr;
}

// ── Unbind ─────────────────────────────────────────────────────────────────

void ServiceLocator::reset() {
    g_registry = nullptr;
    LIZ_INFO("ServiceLocator: reset (unbound)");
}

} // namespace liz
