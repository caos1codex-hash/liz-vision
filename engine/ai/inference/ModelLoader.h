#pragma once

#include "engine/ai/inference/Model.h"

#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace liz {

/// Factory that creates Model instances by name.
///
/// In this sprint: maintains a catalog of known model names and
/// creates pre-configured simulated Model objects.
/// Future: will load real ONNX / TensorRT models from disk.
///
/// Usage:
///   ModelLoader loader;
///   loader.discover();                      // scan available models
///   auto model = loader.load("liz_upscaler_v1");
///   if (model) { ... }
class ModelLoader {
public:
    ModelLoader() = default;

    // -- Discovery ---------------------------------------------------------------
    /// Scan the (simulated) model directory and register all known models.
    /// Logs each model found.
    void discover();

    /// Check whether a model name is known.
    bool has_model(std::string_view name) const;

    /// List all discovered model names.
    std::vector<std::string> available_models() const;

    // -- Loading -----------------------------------------------------------------
    /// Load a model by name. Returns nullptr if the name is unknown.
    /// Logs the loading action and calls model->on_load().
    std::unique_ptr<Model> load(std::string_view name);

    /// Number of models currently discovered.
    std::size_t model_count() const;

private:
    /// A descriptor for a model that can be loaded on demand.
    struct ModelDescriptor {
        std::string    name;
        ModelType      type;
        std::string    version;
        TensorFormat   input_fmt;
        TensorFormat   output_fmt;
    };

    std::vector<ModelDescriptor> catalog_;

    /// Populate the catalog with built-in simulated models.
    void populate_builtin_models();
};

} // namespace liz