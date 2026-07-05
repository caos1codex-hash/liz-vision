#pragma once

#include "engine/config/ConfigValue.h"

#include <cstddef>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace liz {

/// Groups multiple ConfigValue entries under a named section.
///
/// Example sections: Graphics, Audio, Engine, AI, GPU, Diagnostics, etc.
class ConfigSection {
public:
    explicit ConfigSection(std::string name);

    // Non-copyable, movable.
    ConfigSection(const ConfigSection&) = delete;
    ConfigSection& operator=(const ConfigSection&) = delete;
    ConfigSection(ConfigSection&&) noexcept = default;
    ConfigSection& operator=(ConfigSection&&) noexcept = default;
    ~ConfigSection() = default;

    // ── Accessors ────────────────────────────────────────────────────

    const std::string& name() const;

    // ── Value management ─────────────────────────────────────────────

    /// Add a value.  Returns false if key already exists.
    bool add_value(std::unique_ptr<ConfigValue> value);

    /// Remove a value by key.  Returns false if not found.
    bool remove_value(const std::string& key);

    /// Find a value by key.  Returns nullptr if not found.
    const ConfigValue* find(const std::string& key) const;

    /// Find a mutable value by key.  Returns nullptr if not found.
    ConfigValue* find_mutable(const std::string& key);

    /// Check if a key exists.
    bool exists(const std::string& key) const;

    /// List all value keys.
    std::vector<std::string> list() const;

    /// Number of values in this section.
    std::size_t count() const;

    /// Count how many values have been modified.
    std::size_t modified_count() const;

    /// Clear all values.
    void clear();

private:
    std::string name_;
    std::unordered_map<std::string, std::unique_ptr<ConfigValue>> values_;
};

} // namespace liz
