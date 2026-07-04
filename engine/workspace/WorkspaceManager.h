#pragma once

#include "engine/workspace/WorkspaceTypes.h"
#include "engine/workspace/WorkspaceStatistics.h"

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

namespace liz {

class Workspace;
class EventBus;

/// Central manager for the LIZ Vision Workspace system.
///
/// Manages the full lifecycle of workspaces: create, open, close, destroy.
/// Each workspace can contain multiple projects.
/// Integrates with EventBus, Diagnostics, and Service Registry.
/// Everything is in-memory only — no persistence.
class WorkspaceManager {
public:
    WorkspaceManager();
    ~WorkspaceManager() = default;

    // Non-copyable.
    WorkspaceManager(const WorkspaceManager&) = delete;
    WorkspaceManager& operator=(const WorkspaceManager&) = delete;

    // ── Lifecycle ──────────────────────────────────────────────────

    /// Initialize the workspace manager.
    bool initialize();

    /// Shut down the workspace manager. Closes and destroys all workspaces.
    void shutdown();

    bool is_initialized() const { return initialized_; }

    // ── Workspace operations ────────────────────────────────────────

    /// Create a new workspace with the given name. Returns a pointer.
    /// Returns nullptr if a workspace with the same name already exists.
    Workspace* create_workspace(const std::string& name);

    /// Open a workspace by UUID. Returns true on success.
    bool open_workspace(const std::string& uuid);

    /// Close a workspace by UUID. Returns true on success.
    bool close_workspace(const std::string& uuid);

    /// Destroy a workspace by UUID. Removes it completely. Returns true on success.
    bool destroy_workspace(const std::string& uuid);

    /// Set a workspace as the active workspace. Returns true on success.
    bool set_active_workspace(const std::string& uuid);

    // ── Queries ──────────────────────────────────────────────────────

    /// Get the currently active workspace. Returns nullptr if none.
    Workspace* active_workspace() const;

    /// Find a workspace by UUID. Returns nullptr if not found.
    Workspace* find_workspace(const std::string& uuid) const;

    /// Find a workspace by name. Returns nullptr if not found.
    Workspace* find_workspace_by_name(const std::string& name) const;

    /// List all workspace names.
    std::vector<std::string> list_workspaces() const;

    /// Number of managed workspaces.
    std::size_t workspace_count() const { return entries_.size(); }

    // ── Project management within workspaces ─────────────────────────

    /// Add a project UUID to a workspace.
    bool add_project(const std::string& workspace_uuid, const std::string& project_uuid);

    /// Remove a project UUID from a workspace.
    bool remove_project(const std::string& workspace_uuid, const std::string& project_uuid);

    /// List all project UUIDs in a workspace.
    std::vector<std::string> list_projects(const std::string& workspace_uuid) const;

    // ── Statistics ──────────────────────────────────────────────────

    /// Get workspace system statistics.
    WorkspaceStatistics statistics() const;

    // ── EventBus ────────────────────────────────────────────────────

    void set_event_bus(EventBus* bus);

    // ── Bulk ────────────────────────────────────────────────────────

    /// Remove all workspaces.
    void clear();

private:
    void publish_event(int event_code, const std::string& source, const std::string& message);

    struct Entry {
        std::unique_ptr<Workspace> workspace;
    };

    std::vector<Entry> entries_;
    EventBus*          event_bus_   = nullptr;
    bool               initialized_ = false;

    // Active workspace tracking.
    std::string active_workspace_uuid_;

    // Cumulative counters.
    std::size_t workspaces_created_ = 0;
    std::size_t workspaces_open_    = 0;
};

} // namespace liz
