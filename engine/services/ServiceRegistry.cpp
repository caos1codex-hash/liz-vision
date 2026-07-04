#include "engine/services/ServiceRegistry.h"
#include "engine/core/Logger.h"

#include <algorithm>
#include <sstream>

namespace liz {

// ── Constructor ─────────────────────────────────────────────────────────────

ServiceRegistry::ServiceRegistry() = default;

// ── Registration ───────────────────────────────────────────────────────────

bool ServiceRegistry::register_service(const std::string& name,
                                        ServiceType type,
                                        const std::string& version) {
    // Check for duplicates first.
    if (exists(name)) {
        ++duplicate_attempts_;
        std::ostringstream oss;
        oss << "ServiceRegistry: duplicate registration rejected — '" << name << "'";
        LIZ_WARN(oss.str());
        return false;
    }

    auto service = std::make_unique<Service>(name, type, version);
    entries_.push_back(Entry{std::move(service)});
    ++total_registered_;

    std::ostringstream oss;
    oss << "ServiceRegistry: registered '" << name
        << "' (type=" << service_type_to_string(type)
        << ", version=" << version << ")";
    LIZ_INFO(oss.str());

    return true;
}

bool ServiceRegistry::unregister_service(const std::string& name) {
    auto it = std::find_if(entries_.begin(), entries_.end(),
        [&name](const Entry& e) { return e.service->name() == name; });

    if (it == entries_.end()) {
        std::ostringstream oss;
        oss << "ServiceRegistry: unregister failed — '" << name << "' not found";
        LIZ_WARN(oss.str());
        return false;
    }

    std::ostringstream oss;
    oss << "ServiceRegistry: unregistered '" << name << "'";
    LIZ_INFO(oss.str());

    entries_.erase(it);
    ++total_removed_;
    return true;
}

// ── Query ───────────────────────────────────────────────────────────────────

Service* ServiceRegistry::find(const std::string& name) const {
    auto it = std::find_if(entries_.begin(), entries_.end(),
        [&name](const Entry& e) { return e.service->name() == name; });

    return (it != entries_.end()) ? it->service.get() : nullptr;
}

bool ServiceRegistry::exists(const std::string& name) const {
    return find(name) != nullptr;
}

std::size_t ServiceRegistry::count() const {
    return entries_.size();
}

std::vector<std::string> ServiceRegistry::list() const {
    std::vector<std::string> names;
    names.reserve(entries_.size());
    for (const auto& entry : entries_) {
        names.push_back(entry.service->name());
    }
    return names;
}

// ── Bulk operations ─────────────────────────────────────────────────────────

void ServiceRegistry::clear() {
    std::size_t cleared = entries_.size();
    entries_.clear();
    total_removed_ += cleared;

    std::ostringstream oss;
    oss << "ServiceRegistry: cleared " << cleared << " services";
    LIZ_INFO(oss.str());
}

// ── Statistics ──────────────────────────────────────────────────────────────

ServiceRegistryStats ServiceRegistry::statistics() const {
    return ServiceRegistryStats{
        total_registered_,
        total_removed_,
        duplicate_attempts_
    };
}

void ServiceRegistry::log_statistics() const {
    auto stats = statistics();
    std::ostringstream oss;
    oss << "ServiceRegistry stats: "
        << "registered=" << stats.total_registered
        << ", removed=" << stats.total_removed
        << ", duplicates_rejected=" << stats.duplicate_attempts
        << ", active=" << count();
    LIZ_INFO(oss.str());
}

} // namespace liz
