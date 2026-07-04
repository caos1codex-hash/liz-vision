#pragma once

#include "engine/project/ProjectTypes.h"
#include "engine/project/ProjectMetadata.h"
#include "engine/project/ProjectSettings.h"

#include <chrono>
#include <string>

namespace liz {

/// A Project represents a complete LIZ Vision project.
///
/// Contains identity, metadata, settings, and lifecycle state.
/// Everything is in-memory only — no persistence in this sprint.
///
/// Usage:
///   Project project("My Video Project");
///   project.set_description("4K upscale workflow");
///   project.metadata().set_author("LIZ Team");
///   project.settings().set_gpu_enabled(true);
class Project {
public:
    explicit Project(std::string name);
    ~Project() = default;

    // Non-copyable.
    Project(const Project&) = delete;
    Project& operator=(const Project&) = delete;

    // Movable.
    Project(Project&&) noexcept = default;
    Project& operator=(Project&&) noexcept = default;

    // ── Identity ────────────────────────────────────────────────────

    const std::string& uuid() const { return uuid_; }
    const std::string& name() const { return name_; }
    void set_name(const std::string& name) { name_ = name; }

    // ── Description / Author / Version ─────────────────────────────

    const std::string& description() const { return description_; }
    void set_description(const std::string& desc) { description_ = desc; }

    const std::string& author() const { return author_; }
    void set_author(const std::string& author) { author_ = author; }

    const std::string& version() const { return version_; }
    void set_version(const std::string& ver) { version_ = ver; }

    // ── State ──────────────────────────────────────────────────────

    ProjectState state() const { return state_; }
    void set_state(ProjectState state) { state_ = state; }

    // ── Timestamps ──────────────────────────────────────────────────

    const std::chrono::steady_clock::time_point& created_at() const { return created_at_; }
    const std::chrono::steady_clock::time_point& last_modified() const { return last_modified_; }
    void touch_modified() { last_modified_ = std::chrono::steady_clock::now(); }

    // ── Sub-objects ───────────────────────────────────────────────

    ProjectMetadata& metadata() { return metadata_; }
    const ProjectMetadata& metadata() const { return metadata_; }

    ProjectSettings& settings() { return settings_; }
    const ProjectSettings& settings() const { return settings_; }

    // ── Info ──────────────────────────────────────────────────────

    /// Generate a formatted summary string.
    std::string to_string() const;

private:
    static std::string generate_uuid();

    std::string                              uuid_;
    std::string                              name_;
    std::string                              description_;
    std::string                              author_;
    std::string                              version_;
    ProjectState                             state_;
    std::chrono::steady_clock::time_point    created_at_;
    std::chrono::steady_clock::time_point    last_modified_;
    ProjectMetadata                          metadata_;
    ProjectSettings                          settings_;
};

} // namespace liz
