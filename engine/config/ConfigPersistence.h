#pragma once

#include <string>

namespace liz {

class Configuration;

/// Serializes/deserializes a Configuration to/from a textual representation
/// (INI-like, in-memory). No external libraries. Optional file I/O via path.
class ConfigPersistence {
public:
    /// Serialize a Configuration to a textual representation.
    static std::string serialize(const Configuration& config);

    /// Parse a textual representation and merge into an existing Configuration.
    /// Returns true on success. Sections/keys that already exist are overwritten.
    static bool deserialize(const std::string& text, Configuration& config);

    /// Serialize and write to a file path. Returns true on success.
    static bool save_to(const std::string& path, const Configuration& config);

    /// Read a file path and load into an existing Configuration.
    /// Returns true on success.
    static bool load_from(const std::string& path, Configuration& config);
};

} // namespace liz
