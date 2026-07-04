#pragma once

#include <map>
#include <optional>
#include <string>
#include <string_view>

namespace liz {

/// In-memory key-value configuration store.
///
/// Provides global access to engine settings via a singleton.
/// All values are stored as strings — consumers convert to
/// int / bool / float as needed using std::stoi / std::stof etc.
///
/// No filesystem, no parsing — pure in-memory for this sprint.
class Config {
public:
    /// Retrieve the global Config instance.
    static Config& instance();

    /// Set or overwrite a configuration value.
    void set(std::string_view key, std::string_view value);

    /// Get a value by key.  Returns std::nullopt if not found.
    std::optional<std::string> get(std::string_view key) const;

    /// Get a value by key with a fallback default.
    std::string get(std::string_view key, std::string_view default_value) const;

    /// Check whether a key exists in the store.
    bool has(std::string_view key) const;

    /// Remove a key from the store.  Returns true if it existed.
    bool remove(std::string_view key);

    /// Number of entries currently stored.
    std::size_t size() const;

    /// Load the hardcoded defaults defined by the engine.
    void load_defaults();

private:
    Config() = default;

    std::map<std::string, std::string> data_;
};

} // namespace liz