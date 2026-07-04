#pragma once

#include "engine/project/ProjectTypes.h"
#include "engine/project/ProjectStatistics.h"

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

namespace liz {

class Project;
class EventBus;
class Logger;

/// Central manager for the LIZ Vision Project system.
///
/// Manages the full lifecycle of projects: create, open, save, close, destroy.
/// Integrates with Logger, EventBus, Diagnostics, and Service Registry.
/// Everything is in-memory only — no persistence.
///
/// Usage:
///   ProjectManager mgr;
///   mgr.initialize();
///   auto* proj = mgr.create_project("My Project");
///   mgr.open_project(proj->uuid());
///   mgr.save_project(proj->uuid());
///   mgr.close_project(proj->uuid());
class ProjectManager {
public:
    ProjectManager();
    ~ProjectManager() = default;

    // Non-copyable.
    ProjectManager(const ProjectManager&) = delete;
    ProjectManager& operator=(const ProjectManager&) = delete;

    // ── Lifecycle ──────────────────────────────────────────────────

    /// Initialize the project manager.
    bool initialize();

    /// Shut down the project manager.  Closes and destroys all projects.
    void shutdown();

    bool is_initialized() const { return initialized_; }

    // ── Project operations ──────────────────────────────────────────

    /// Create a new project with the given name.  Returns a pointer to the project.
    /// Returns nullptr if a project with the same name already exists.
    Project* create_project(const std::string& name);

    /// Open a project by UUID.  Returns true on success.
    bool open_project(const std::string& uuid);

    /// Save the current state of a project (simulated).  Returns true on success.
    bool save_project(const std::string& uuid);

    /// Close a project by UUID.  Returns true on success.
    bool close_project(const std::string& uuid);

    /// Destroy a project by UUID.  Removes it completely.  Returns true on success.
    bool destroy_project(const std::string& uuid);

    // ── Queries ──────────────────────────────────────────────────────

    /// Get the currently active (last opened) project.  Returns nullptr if none.
    Project* current_project() const;

    /// Find a project by UUID.  Returns nullptr if not found.
    Project* find_project(const std::string& uuid) const;

    /// Find a project by name.  Returns nullptr if not found.
    Project* find_project_by_name(const std::string& name) const;

    /// List all project names.
    std::vector<std::string> list_projects() const;

    /// Number of managed projects.
    std::size_t project_count() const { return entries_.size(); }

    // ── Statistics ──────────────────────────────────────────────────

    /// Get project system statistics.
    ProjectStatistics statistics() const;

    // ── EventBus ────────────────────────────────────────────────────

    void set_event_bus(EventBus* bus);

    // ── Bulk ────────────────────────────────────────────────────────

    /// Remove all projects.
    void clear();

private:
    void publish_event(int event_code, const std::string& source, const std::string& message);

    struct Entry {
        std::unique_ptr<Project> project;
    };

    std::vector<Entry> entries_;
    EventBus*          event_bus_   = nullptr;
    bool               initialized_ = false;

    // Current project tracking.
    std::string current_project_uuid_;

    // Cumulative counters.
    std::size_t projects_created_ = 0;
    std::size_t projects_open_    = 0;
    std::size_t projects_saved_   = 0;
    std::size_t projects_closed_  = 0;
};

} // namespace liz
