#include "engine/project/ProjectManager.h"
#include "engine/project/Project.h"
#include "engine/project/ProjectTypes.h"
#include "engine/core/Logger.h"
#include "engine/events/EventBus.h"
#include "engine/events/Event.h"
#include "engine/events/EventType.h"

#include <algorithm>
#include <sstream>

namespace liz {

// ── Constructor ────────────────────────────────────────────────────

ProjectManager::ProjectManager() = default;

// ── Lifecycle ──────────────────────────────────────────────────────

bool ProjectManager::initialize() {
    if (initialized_) return true;

    LIZ_INFO("ProjectManager: initializing");
    initialized_ = true;

    LIZ_INFO("ProjectManager: ready");
    return true;
}

void ProjectManager::shutdown() {
    if (!initialized_) return;

    LIZ_INFO("ProjectManager: shutting down");

    // Close and destroy all projects.
    for (auto& entry : entries_) {
        if (entry.project) {
            if (entry.project->state() != ProjectState::Closed) {
                entry.project->set_state(ProjectState::Closed);
            }
        }
    }
    entries_.clear();
    current_project_uuid_.clear();

    initialized_ = false;
    LIZ_INFO("ProjectManager: stopped");
}

// ── Project operations ────────────────────────────────────────────

Project* ProjectManager::create_project(const std::string& name) {
    if (!initialized_) {
        LIZ_ERROR("ProjectManager: not initialized");
        return nullptr;
    }

    // Check for duplicate name.
    if (find_project_by_name(name)) {
        LIZ_WARN("ProjectManager: project already exists with name");
        return nullptr;
    }

    auto project = std::make_unique<Project>(name);
    Project* raw = project.get();

    std::ostringstream oss;
    oss << "ProjectManager: project created — uuid=" << raw->uuid()
        << " name=" << name;
    LIZ_INFO(oss.str());

    entries_.push_back(Entry{std::move(project)});
    current_project_uuid_ = raw->uuid();

    ++projects_created_;
    raw->set_state(ProjectState::Created);
    raw->touch_modified();

    publish_event(0, "ProjectManager", "ProjectCreated: " + name);

    return raw;
}

bool ProjectManager::open_project(const std::string& uuid) {
    if (!initialized_) return false;

    auto* project = find_project(uuid);
    if (!project) {
        LIZ_WARN("ProjectManager: open_project — not found");
        return false;
    }

    if (project->state() == ProjectState::Closed ||
        project->state() == ProjectState::Created) {
        project->set_state(ProjectState::Opened);
        project->touch_modified();
    }

    current_project_uuid_ = uuid;
    ++projects_open_;

    std::ostringstream oss;
    oss << "ProjectManager: project opened — uuid=" << uuid
        << " name=" << project->name();
    LIZ_INFO(oss.str());

    publish_event(1, "ProjectManager", "ProjectOpened: " + project->name());

    return true;
}

bool ProjectManager::save_project(const std::string& uuid) {
    if (!initialized_) return false;

    auto* project = find_project(uuid);
    if (!project) {
        LIZ_WARN("ProjectManager: save_project — not found");
        return false;
    }

    project->set_state(ProjectState::Saved);
    project->touch_modified();
    ++projects_saved_;

    std::ostringstream oss;
    oss << "ProjectManager: project saved — uuid=" << uuid
        << " name=" << project->name();
    LIZ_INFO(oss.str());

    publish_event(2, "ProjectManager", "ProjectSaved: " + project->name());

    return true;
}

bool ProjectManager::close_project(const std::string& uuid) {
    if (!initialized_) return false;

    auto* project = find_project(uuid);
    if (!project) {
        LIZ_WARN("ProjectManager: close_project — not found");
        return false;
    }

    project->set_state(ProjectState::Closed);
    project->touch_modified();
    ++projects_closed_;

    std::ostringstream oss;
    oss << "ProjectManager: project closed — uuid=" << uuid
        << " name=" << project->name();
    LIZ_INFO(oss.str());

    publish_event(3, "ProjectManager", "ProjectClosed: " + project->name());

    // If this was the current project, clear the reference.
    if (current_project_uuid_ == uuid) {
        current_project_uuid_.clear();
    }

    return true;
}

bool ProjectManager::destroy_project(const std::string& uuid) {
    if (!initialized_) return false;

    auto it = std::find_if(entries_.begin(), entries_.end(),
        [&uuid](const Entry& e) { return e.project && e.project->uuid() == uuid; });

    if (it == entries_.end()) {
        LIZ_WARN("ProjectManager: destroy_project — not found");
        return false;
    }

    std::string name = it->project->name();

    std::ostringstream oss;
    oss << "ProjectManager: project destroyed — uuid=" << uuid
        << " name=" << name;
    LIZ_INFO(oss.str());

    publish_event(4, "ProjectManager", "ProjectDestroyed: " + name);

    entries_.erase(it);

    if (current_project_uuid_ == uuid) {
        current_project_uuid_.clear();
    }

    return true;
}

// ── Queries ──────────────────────────────────────────────────────────

Project* ProjectManager::current_project() const {
    if (current_project_uuid_.empty()) return nullptr;
    return find_project(current_project_uuid_);
}

Project* ProjectManager::find_project(const std::string& uuid) const {
    for (const auto& entry : entries_) {
        if (entry.project && entry.project->uuid() == uuid) {
            return entry.project.get();
        }
    }
    return nullptr;
}

Project* ProjectManager::find_project_by_name(const std::string& name) const {
    for (const auto& entry : entries_) {
        if (entry.project && entry.project->name() == name) {
            return entry.project.get();
        }
    }
    return nullptr;
}

std::vector<std::string> ProjectManager::list_projects() const {
    std::vector<std::string> names;
    names.reserve(entries_.size());
    for (const auto& entry : entries_) {
        if (entry.project) {
            names.push_back(entry.project->name());
        }
    }
    return names;
}

// ── Statistics ──────────────────────────────────────────────────────

ProjectStatistics ProjectManager::statistics() const {
    ProjectStatistics stats;
    stats.projects_created = projects_created_;
    stats.projects_open    = projects_open_;
    stats.projects_saved  = projects_saved_;
    stats.projects_closed = projects_closed_;
    stats.active_project  = current_project_uuid_.empty() ? 0 : 1;
    stats.assets          = 0;  // Will be populated by integration layer.
    stats.pipelines       = 0;  // Will be populated by integration layer.
    stats.services        = 0;  // Will be populated by integration layer.
    stats.runtime_seconds = 0.0;
    return stats;
}

// ── EventBus ─────────────────────────────────────────────────────────

void ProjectManager::set_event_bus(EventBus* bus) {
    event_bus_ = bus;
}

void ProjectManager::publish_event(int event_code, const std::string& source, const std::string& message) {
    if (!event_bus_) return;

    EventType type;
    switch (event_code) {
        case 0: type = EventType::ProjectCreated;   break;
        case 1: type = EventType::ProjectOpened;    break;
        case 2: type = EventType::ProjectSaved;     break;
        case 3: type = EventType::ProjectClosed;     break;
        case 4: type = EventType::ProjectDestroyed;   break;
        default: type = EventType::Custom;           break;
    }

    event_bus_->publish(Event(type, source, message));
}

// ── Bulk ────────────────────────────────────────────────────────────

void ProjectManager::clear() {
    entries_.clear();
    current_project_uuid_.clear();
    LIZ_INFO("ProjectManager: all projects cleared");
}

} // namespace liz
