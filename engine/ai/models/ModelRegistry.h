#pragma once

#include "engine/ai/inference/Model.h"

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace liz {

/// A lightweight catalog of all models known to the engine.
///
/// Unlike ModelLoader (which creates Model instances), ModelRegistry
/// is purely a directory: it stores descriptors and allows querying
/// by name, type, or listing all entries.
///
/// The registry is populated once at startup and serves as the
/// single source of truth for "what models exist".
class ModelRegistry {
public:
    ModelRegistry() = default;

    // -- Registration ------------------------------------------------------------
    /// Register a model descriptor by name, type, and version.
    /// Returns false if a model with the same name already exists.
    bool register_model(std::string name,
                        ModelType type,
                        std::string version);

    // -- Query -------------------------------------------------------------------
    /// Check if a model name is registered.
    bool has_model(std::string_view name) const;

    /// Get the list of all registered model names.
    std::vector<std::string> get_model_list() const;

    /// Get model names filtered by type.
    std::vector<std::string> get_models_by_type(ModelType type) const;

    /// Total number of registered models.
    std::size_t size() const;

    /// Log all registered models.
    void log_all() const;

private:
    struct Entry {
        std::string name;
        ModelType   type;
        std::string version;
    };

    std::vector<Entry> entries_;
};

} // namespace liz