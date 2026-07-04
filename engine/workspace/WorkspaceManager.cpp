#include "engine/workspace/WorkspaceManager.h"
#include "engine/workspace/Workspace.h"
#include "engine/workspace/WorkspaceTypes.h"
#include "engine/core/Logger.h"
#include "engine/events/EventBus.h"
#include "engine/events/Event.h"
#include "engine/events/EventType.h"

#include <algorithm>
#include <sstream>

namespace liz {

// ── Constructor ────────────────────────────────────────────────────

WorkspaceManager::WorkspaceManager() = default;

// ── Lifecycle ──────────────────────────────────────────────────────

bool WorkspaceManager::initialize() {
    if (initialized_) return true;

    LIZ_INFO("WorkspaceManager: initializing");
    initialized_ = true;

    LIZ_INFO("WorkspaceManager: ready");
    return true;
}

void WorkspaceManager::shutdown() {
    if (!initialized_) return;

    LIZ_INFO("WorkspaceManager: shutting down");

    for (auto& entry : entries_) {
        if (entry.workspace) {
            if (entry.workspace->state() != WorkspaceState::Closed) {
                entry.workspace->set_state(WorkspaceState::Closed);
            }
        }
    }
    entries_.clear();
    active_workspace_uuid_.clear();

    initialized_ = false;
    LIZ_INFO("WorkspaceManager: stopped");
}

// ── Workspace operations ────────────────────────────────────────────

Workspace* WorkspaceManager::create_workspace(const std::string& name) {
    if (!initialized_) {
        LIZ_ERROR("WorkspaceManager: not initialized");
        return nullptr;
    }

    // Check for duplicate name.
    if (find_workspace_by_name(name)) {
        LIZ_WARN("WorkspaceManager: workspace already exists with name");
        return nullptr;
    }

    auto workspace = std::make_unique<Workspace>(name);
    Workspace* raw = workspace.get();

    std::ostringstream oss;
    oss << "WorkspaceManager: workspace created — uuid=" << raw->uuid()
        << " name=" << name;
    LIZ_INFO(oss.str());

    entries_.push_back(Entry{std::move(workspace)});
    ++workspaces_created_;

    publish_event(0, "WorkspaceManager", "WorkspaceCreated: " + name);

    return raw;
}

bool WorkspaceManager::open_workspace(const std::string& uuid) {
    if (!initialized_) return false;

    auto* ws = find_workspace(uuid);
    if (!ws) {
        LIZ_WARN("WorkspaceManager: open_workspace — not found");
        return false;
    }

    ws->set_state(WorkspaceState::Opened);
    ws->touch_modified();
    ++workspaces_open_;

    std::ostringstream oss;
    oss << "WorkspaceManager: workspace opened — uuid=" << uuid
        << " name=" << ws->name();
    LIZ_INFO(oss.str());

    publish_event(1, "WorkspaceManager", "WorkspaceOpened: " + ws->name());

    return true;
}

bool WorkspaceManager::close_workspace(const std::string& uuid) {
    if (!initialized_) return false;

    auto* ws = find_workspace(uuid);
    if (!ws) {
        LIZ_WARN("WorkspaceManager: close_workspace — not found");
        return false;
    }

    ws->set_state(WorkspaceState::Closed);
    ws->touch_modified();

    std::ostringstream oss;
    oss << "WorkspaceManager: workspace closed — uuid=" << uuid
        << " name=" << ws->name();
    LIZ_INFO(oss.str());

    publish_event(3, "WorkspaceManager", "WorkspaceClosed: " + ws->name());

    if (active_workspace_uuid_ == uuid) {
        active_workspace_uuid_.clear();
    }

    return true;
}

bool WorkspaceManager::destroy_workspace(const std::string& uuid) {
    if (!initialized_) return false;

    auto it = std::find_if(entries_.begin(), entries_.end(),
        [&uuid](const Entry& e) { return e.workspace && e.workspace->uuid() == uuid; });

    if (it == entries_.end()) {
        LIZ_WARN("WorkspaceManager: destroy_workspace — not found");
        return false;
    }

    std::string name = it->workspace->name();

    std::ostringstream oss;
    oss << "WorkspaceManager: workspace destroyed — uuid=" << uuid
        << " name=" << name;
    LIZ_INFO(oss.str());

    publish_event(4, "WorkspaceManager", "WorkspaceDestroyed: " + name);

    entries_.erase(it);

    if (active_workspace_uuid_ == uuid) {
        active_workspace_uuid_.clear();
    }

    return true;
}

bool WorkspaceManager::set_active_workspace(const std::string& uuid) {
    if (!initialized_) return false;

    auto* ws = find_workspace(uuid);
    if (!ws) {
        LIZ_WARN("WorkspaceManager: set_active_workspace — not found");
        return false;
    }

    ws->set_state(WorkspaceState::Active);
    ws->touch_modified();
    active_workspace_uuid_ = uuid;

    std::ostringstream oss;
    oss << "WorkspaceManager: active workspace set — uuid=" << uuid
        << " name=" << ws->name();
    LIZ_INFO(oss.str());

    publish_event(2, "WorkspaceManager", "WorkspaceActivated: " + ws->name());

    return true;
}

// ── Queries ──────────────────────────────────────────────────────────

Workspace* WorkspaceManager::active_workspace() const {
    if (active_workspace_uuid_.empty()) return nullptr;
    return find_workspace(active_workspace_uuid_);
}

Workspace* WorkspaceManager::find_workspace(const std::string& uuid) const {
    for (const auto& entry : entries_) {
        if (entry.workspace && entry.workspace->uuid() == uuid) {
            return entry.workspace.get();
        }
    }
    return nullptr;
}

Workspace* WorkspaceManager::find_workspace_by_name(const std::string& name) const {
    for (const auto& entry : entries_) {
        if (entry.workspace && entry.workspace->name() == name) {
            return entry.workspace.get();
        }
    }
    return nullptr;
}

std::vector<std::string> WorkspaceManager::list_workspaces() const {
    std::vector<std::string> names;
    names.reserve(entries_.size());
    for (const auto& entry : entries_) {
        if (entry.workspace) {
            names.push_back(entry.workspace->name());
        }
    }
    return names;
}

// ── Project management within workspaces ─────────────────────────────

bool WorkspaceManager::add_project(const std::string& workspace_uuid, const std::string& project_uuid) {
    auto* ws = find_workspace(workspace_uuid);
    if (!ws) return false;

    ws->add_project(project_uuid);
    ws->touch_modified();

    std::ostringstream oss;
    oss << "WorkspaceManager: project added — workspace=" << ws->name()
        << " project_uuid=" << project_uuid;
    LIZ_INFO(oss.str());

    publish_event(5, "WorkspaceManager", "ProjectAddedToWorkspace: " + project_uuid);

    return true;
}

bool WorkspaceManager::remove_project(const std::string& workspace_uuid, const std::string& project_uuid) {
    auto* ws = find_workspace(workspace_uuid);
    if (!ws) return false;

    if (!ws->remove_project(project_uuid)) return false;
    ws->touch_modified();

    std::ostringstream oss;
    oss << "WorkspaceManager: project removed — workspace=" << ws->name()
        << " project_uuid=" << project_uuid;
    LIZ_INFO(oss.str());

    publish_event(6, "WorkspaceManager", "ProjectRemovedFromWorkspace: " + project_uuid);

    return true;
}

std::vector<std::string> WorkspaceManager::list_projects(const std::string& workspace_uuid) const {
    auto* ws = find_workspace(workspace_uuid);
    if (!ws) return {};

    return ws->project_uuids();
}

// ── Statistics ──────────────────────────────────────────────────────

WorkspaceStatistics WorkspaceManager::statistics() const {
    WorkspaceStatistics stats;
    stats.workspaces_created = workspaces_created_;
    stats.workspaces_open    = workspaces_open_;
    stats.active_workspace  = active_workspace_uuid_.empty() ? 0 : 1;
    stats.projects_loaded   = 0;
    stats.assets_loaded     = 0;
    stats.pipelines_loaded  = 0;
    stats.runtime_seconds  = 0.0;

    // Count total projects across all workspaces.
    for (const auto& entry : entries_) {
        if (entry.workspace) {
            stats.projects_loaded += entry.workspace->project_count();
        }
    }

    return stats;
}

// ── EventBus ─────────────────────────────────────────────────────────

void WorkspaceManager::set_event_bus(EventBus* bus) {
    event_bus_ = bus;
}

void WorkspaceManager::publish_event(int event_code, const std::string& source, const std::string& message) {
    if (!event_bus_) return;

    EventType type;
    switch (event_code) {
        case 0: type = EventType::WorkspaceCreated;              break;
        case 1: type = EventType::WorkspaceOpened;               break;
        case 2: type = EventType::WorkspaceActivated;            break;
        case 3: type = EventType::WorkspaceClosed;               break;
        case 4: type = EventType::WorkspaceDestroyed;            break;
        case 5: type = EventType::ProjectAddedToWorkspace;       break;
        case 6: type = EventType::ProjectRemovedFromWorkspace;   break;
        default: type = EventType::Custom;                         break;
    }

    event_bus_->publish(Event(type, source, message));
}

// ── Bulk ────────────────────────────────────────────────────────────

void WorkspaceManager::clear() {
    entries_.clear();
    active_workspace_uuid_.clear();
    LIZ_INFO("WorkspaceManager: all workspaces cleared");
}

} // namespace liz
