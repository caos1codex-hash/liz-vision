#include "demo/DemoRunner.h"
#include "engine/core/Logger.h"

#include <algorithm>
#include <sstream>

// ── DemoRegistry ────────────────────────────────────────────────────────

bool DemoRegistry::register_demo(std::unique_ptr<DemoRunner> demo) {
    if (!demo) return false;

    const auto& name = demo->name();

    // Check for duplicates.
    auto it = std::find_if(entries_.begin(), entries_.end(),
        [&name](const Entry& e) { return e.demo->name() == name; });

    if (it != entries_.end()) {
        std::ostringstream oss;
        oss << "DemoRegistry: demo '" << name << "' already registered";
        LIZ_WARN(oss.str());
        return false;
    }

    entries_.push_back(Entry{std::move(demo)});

    std::ostringstream oss;
    oss << "DemoRegistry: registered '" << name << "'";
    LIZ_INFO(oss.str());

    return true;
}

bool DemoRegistry::remove_demo(const std::string& name) {
    auto it = std::find_if(entries_.begin(), entries_.end(),
        [&name](const Entry& e) { return e.demo->name() == name; });

    if (it == entries_.end()) {
        return false;
    }

    entries_.erase(it);

    std::ostringstream oss;
    oss << "DemoRegistry: removed '" << name << "'";
    LIZ_INFO(oss.str());

    return true;
}

void DemoRegistry::clear() {
    entries_.clear();
    LIZ_INFO("DemoRegistry: all demos cleared");
}

int DemoRegistry::run_demo(const std::string& name) {
    for (auto& entry : entries_) {
        if (entry.demo->name() == name) {
            std::ostringstream oss;
            oss << "DemoRegistry: running '" << name << "'...";
            LIZ_INFO(oss.str());
            return entry.demo->run();
        }
    }

    std::ostringstream oss;
    oss << "DemoRegistry: demo '" << name << "' not found";
    LIZ_WARN(oss.str());
    return -1;
}

int DemoRegistry::run_all() {
    int result = 0;
    for (auto& entry : entries_) {
        int r = entry.demo->run();
        if (r != 0) {
            result = r;
        }
    }
    return result;
}

std::vector<std::string> DemoRegistry::list() const {
    std::vector<std::string> names;
    for (const auto& entry : entries_) {
        names.push_back(entry.demo->name());
    }
    return names;
}

std::size_t DemoRegistry::count() const {
    return entries_.size();
}

bool DemoRegistry::has(const std::string& name) const {
    return std::any_of(entries_.begin(), entries_.end(),
        [&name](const Entry& e) { return e.demo->name() == name; });
}
