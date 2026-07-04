#pragma once

#include <cstddef>
#include <string>

namespace liz {

/// User preferences for a Workspace.
///
/// All fields are in-memory only — no persistence.
class WorkspacePreferences {
public:
    WorkspacePreferences() = default;
    ~WorkspacePreferences() = default;

    // ── Accessors ────────────────────────────────────────────────────

    const std::string& theme() const { return theme_; }
    const std::string& language() const { return language_; }
    bool autosave_enabled() const { return autosave_enabled_; }
    bool gpu_enabled() const { return gpu_enabled_; }
    bool diagnostics_enabled() const { return diagnostics_enabled_; }
    bool auto_validate_pipeline() const { return auto_validate_pipeline_; }
    std::size_t thread_count() const { return thread_count_; }
    std::size_t batch_size() const { return batch_size_; }
    std::size_t recent_projects_limit() const { return recent_projects_limit_; }

    // ── Mutators ────────────────────────────────────────────────────

    void set_theme(const std::string& value) { theme_ = value; }
    void set_language(const std::string& value) { language_ = value; }
    void set_autosave_enabled(bool value) { autosave_enabled_ = value; }
    void set_gpu_enabled(bool value) { gpu_enabled_ = value; }
    void set_diagnostics_enabled(bool value) { diagnostics_enabled_ = value; }
    void set_auto_validate_pipeline(bool value) { auto_validate_pipeline_ = value; }
    void set_thread_count(std::size_t value) { thread_count_ = value; }
    void set_batch_size(std::size_t value) { batch_size_ = value; }
    void set_recent_projects_limit(std::size_t value) { recent_projects_limit_ = value; }

    /// Generate a formatted summary string.
    std::string to_string() const;

private:
    std::string  theme_                   = "dark";
    std::string  language_                = "en";
    bool         autosave_enabled_        = false;
    bool         gpu_enabled_             = true;
    bool         diagnostics_enabled_    = true;
    bool         auto_validate_pipeline_ = true;
    std::size_t  thread_count_            = 4;
    std::size_t  batch_size_              = 8;
    std::size_t  recent_projects_limit_  = 10;
};

} // namespace liz
