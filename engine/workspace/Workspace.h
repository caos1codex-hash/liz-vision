#pragma once

#include "engine/workspace/WorkspaceTypes.h"
#include "engine/workspace/WorkspaceLayout.h"
#include "engine/workspace/WorkspacePreferences.h"

#include <chrono>
#include <string>
#include <vector>

namespace liz {

/// A Workspace represents the complete working environment for a user session.
///
/// Contains a set of projects, layout, preferences, and lifecycle state.
/// Everything is in-memory only — no persistence.
class Workspace {
public:
    explicit Workspace(std::string name);
    ~Workspace() = default;

    // Non-copyable.
    Workspace(const Workspace&) = delete;
    Workspace& operator=(const Workspace&) = delete;

    // Movable.
    Workspace(Workspace&&) noexcept = default;
    Workspace& operator=(Workspace&&) noexcept = default;

    // ── Identity ────────────────────────────────────────────────────

    const std::string& uuid() const { return uuid_; }
    const std::string& name() const { return name_; }
    void set_name(const std::string& name) { name_ = name; }

    // ── State ──────────────────────────────────────────────────────

    WorkspaceState state() const { return state_; }
    void set_state(WorkspaceState state) { state_ = state; }

    // ── Timestamps ──────────────────────────────────────────────────

    const std::chrono::steady_clock::time_point& created_at() const { return created_at_; }
    const std::chrono::steady_clock::time_point& last_modified() const { return last_modified_; }
    void touch_modified() { last_modified_ = std::chrono::steady_clock::now(); }

    // ── Project list ───────────────────────────────────────────────

    /// Add a project UUID to this workspace.
    void add_project(const std::string& project_uuid) { project_uuids_.push_back(project_uuid); }

    /// Remove a project UUID from this workspace.
    bool remove_project(const std::string& project_uuid);

    /// Get all project UUIDs in this workspace.
    const std::vector<std::string>& project_uuids() const { return project_uuids_; }

    /// Number of projects in this workspace.
    std::size_t project_count() const { return project_uuids_.size(); }

    // ── Sub-objects ───────────────────────────────────────────────

    WorkspaceLayout& layout() { return layout_; }
    const WorkspaceLayout& layout() const { return layout_; }

    WorkspacePreferences& preferences() { return preferences_; }
    const WorkspacePreferences& preferences() const { return preferences_; }

    // ── Info ──────────────────────────────────────────────────────

    /// Generate a formatted summary string.
    std::string to_string() const;

private:
    static std::string generate_uuid();

    std::string                              uuid_;
    std::string                              name_;
    WorkspaceState                           state_;
    std::chrono::steady_clock::time_point    created_at_;
    std::chrono::steady_clock::time_point    last_modified_;
    std::vector<std::string>                 project_uuids_;
    WorkspaceLayout                          layout_;
    WorkspacePreferences                      preferences_;
};

} // namespace liz
