#include "engine/services/Service.h"

#include <chrono>
#include <cstdint>
#include <random>
#include <sstream>

namespace liz {

// ── Constructors ──────────────────────────────────────────────────────────

Service::Service()
    : uuid_(generate_uuid())
    , name_("Unnamed")
    , type_(ServiceType::Unknown)
    , version_("0.0.0")
    , state_(ServiceState::Created) {}

Service::Service(std::string name, ServiceType type, std::string version)
    : uuid_(generate_uuid())
    , name_(std::move(name))
    , type_(type)
    , version_(std::move(version))
    , state_(ServiceState::Created) {}

// ── Accessors ──────────────────────────────────────────────────────────────

const std::string& Service::uuid() const { return uuid_; }
const std::string& Service::name() const { return name_; }
ServiceType        Service::type() const { return type_; }
const std::string& Service::version() const { return version_; }
ServiceState       Service::state() const { return state_; }

// ── State transitions ──────────────────────────────────────────────────────

bool Service::initialize() {
    if (state_ != ServiceState::Created) {
        return false;
    }
    state_ = ServiceState::Initialized;
    return true;
}

bool Service::start() {
    if (state_ != ServiceState::Initialized) {
        return false;
    }
    state_ = ServiceState::Running;
    return true;
}

bool Service::stop() {
    if (state_ != ServiceState::Running) {
        return false;
    }
    state_ = ServiceState::Stopped;
    return true;
}

bool Service::destroy() {
    if (state_ == ServiceState::Running) {
        return false;
    }
    state_ = ServiceState::Destroyed;
    return true;
}

// ── Utilities ──────────────────────────────────────────────────────────────

std::string Service::info() const {
    std::ostringstream oss;
    oss << "Service[" << name_
        << "] type=" << service_type_to_string(type_)
        << " version=" << version_
        << " state=" << service_state_to_string(state_)
        << " uuid=" << uuid_;
    return oss.str();
}

// ── Private ────────────────────────────────────────────────────────────────

std::string Service::generate_uuid() {
    static constexpr int kLength = 8;

    auto seed = static_cast<std::uint64_t>(
        std::chrono::steady_clock::now().time_since_epoch().count());
    std::mt19937_64 rng(seed);
    std::uniform_int_distribution<std::uint64_t> dist(0, 0xFFFF'FFFF);

    char buf[kLength + 1];
    for (int i = 0; i < kLength; ++i) {
        std::snprintf(buf + i, 2, "%x", static_cast<int>(dist(rng) & 0xF));
    }
    buf[kLength] = '\0';
    return std::string(buf);
}

} // namespace liz
