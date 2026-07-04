#pragma once

#include "engine/services/ServiceType.h"

#include <cstdint>
#include <string>

namespace liz {

/// Base class for all services in the LIZ Vision engine.
///
/// Each service has a unique identifier (UUID), a human-readable name,
/// a typed classification (ServiceType), a version string, and a
/// lifecycle state (ServiceState).
///
/// This class is NOT polymorphic (no virtual methods) — it is a pure
/// data holder.  Actual service implementations compose this class
/// or are registered alongside it in the ServiceRegistry.
class Service {
public:
    Service();
    Service(std::string name, ServiceType type, std::string version);
    ~Service() = default;

    // Non-copyable (UUID is unique).
    Service(const Service&) = delete;
    Service& operator=(const Service&) = delete;

    // Movable.
    Service(Service&&) noexcept = default;
    Service& operator=(Service&&) noexcept = default;

    // ── Accessors ─────────────────────────────────────────────────────────────

    /// Unique identifier for this service instance.
    const std::string& uuid() const;

    /// Human-readable name (e.g. "Logger", "GPUContext").
    const std::string& name() const;

    /// Typed classification.
    ServiceType type() const;

    /// Semantic version string (e.g. "1.0.0").
    const std::string& version() const;

    /// Current lifecycle state.
    ServiceState state() const;

    // ── State transitions ───────────────────────────────────────────────────

    /// Transition to Initialized.  Returns false if not in Created state.
    bool initialize();

    /// Transition to Running.  Returns false if not in Initialized state.
    bool start();

    /// Transition to Stopped.  Returns false if not in Running state.
    bool stop();

    /// Transition to Destroyed.  Returns false if in Running state.
    bool destroy();

    // ── Utilities ───────────────────────────────────────────────────────────

    /// Generate a summary string for logging.
    std::string info() const;

private:
    /// Generate a short random UUID (8 hex chars).
    static std::string generate_uuid();

    std::string   uuid_;
    std::string   name_;
    ServiceType   type_     = ServiceType::Unknown;
    std::string   version_;
    ServiceState  state_    = ServiceState::Created;
};

} // namespace liz
