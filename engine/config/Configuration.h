#pragma once

#include "engine/config/ConfigSection.h"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace liz {

/// Represents a complete configuration profile.
///
/// Contains UUID, name, description, a list of sections,
/// and creation/modification timestamps.
class Configuration {
public:
    Configuration(std::string name, std::string description = "");

    // Non-copyable, movable.
    Configuration(const Configuration&) = delete;
    Configuration& operator=(const Configuration&) = delete;
    Configuration(Configuration&&) noexcept = default;
    Configuration& operator=(Configuration&&) noexcept = default;
    ~Configuration() = default;

    // ── Accessors ────────────────────────────────────────────────────

    const std::string& uuid() const;
    const std::string& name() const;
    const std::string& description() const;
    std::uint64_t creation_time() const;
    std::uint64_t modification_time() const;

    // ── Section management ──────────────────────────────────────────

    /// Create a new section.  Returns nullptr if already exists.
    ConfigSection* create_section(const std::string& name);

    /// Remove a section by name.  Returns false if not found.
    bool remove_section(const std::string& name);

    /// Find a section by name.  Returns nullptr if not found.
    ConfigSection* find_section(const std::string& name);
    const ConfigSection* find_section(const std::string& name) const;

    /// List all section names.
    std::vector<std::string> list_sections() const;

    /// Total number of sections.
    std::size_t section_count() const;

    /// Total number of values across all sections.
    std::size_t total_values() const;

    /// Total number of modified values across all sections.
    std::size_t total_modified() const;

    /// Clear all sections.
    void clear();

    /// Touch the modification timestamp.
    void touch();

    /// Generate a formatted summary string.
    std::string to_string() const;

private:
    static std::string generate_uuid();

    std::string  uuid_;
    std::string  name_;
    std::string  description_;
    std::uint64_t creation_time_ = 0;
    std::uint64_t modification_time_ = 0;
    std::unordered_map<std::string, std::unique_ptr<ConfigSection>> sections_;
};

} // namespace liz
