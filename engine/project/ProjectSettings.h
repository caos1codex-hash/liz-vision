#pragma once

#include <cstddef>
#include <string>

namespace liz {

/// Configuration settings for a Project.
///
/// Controls engine behavior within the project context.
/// All fields are in-memory only — no persistence.
class ProjectSettings {
public:
    ProjectSettings() = default;
    ~ProjectSettings() = default;

    // ── Accessors ────────────────────────────────────────────────────

    bool gpu_enabled() const { return gpu_enabled_; }
    bool diagnostics_enabled() const { return diagnostics_enabled_; }
    bool pipeline_auto_validation() const { return pipeline_auto_validation_; }
    bool autosave_enabled() const { return autosave_enabled_; }
    std::size_t thread_count() const { return thread_count_; }
    const std::string& language() const { return language_; }
    const std::string& theme() const { return theme_; }
    std::size_t batch_size() const { return batch_size_; }

    // ── Mutators ────────────────────────────────────────────────────

    void set_gpu_enabled(bool value) { gpu_enabled_ = value; }
    void set_diagnostics_enabled(bool value) { diagnostics_enabled_ = value; }
    void set_pipeline_auto_validation(bool value) { pipeline_auto_validation_ = value; }
    void set_autosave_enabled(bool value) { autosave_enabled_ = value; }
    void set_thread_count(std::size_t value) { thread_count_ = value; }
    void set_language(const std::string& value) { language_ = value; }
    void set_theme(const std::string& value) { theme_ = value; }
    void set_batch_size(std::size_t value) { batch_size_ = value; }

    /// Generate a formatted summary string.
    std::string to_string() const;

private:
    bool         gpu_enabled_             = true;
    bool         diagnostics_enabled_    = true;
    bool         pipeline_auto_validation_ = true;
    bool         autosave_enabled_        = false;
    std::size_t  thread_count_            = 4;
    std::string  language_                = "en";
    std::string  theme_                   = "dark";
    std::size_t  batch_size_              = 8;
};

} // namespace liz
