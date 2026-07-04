#include "engine/ai/models/ModelRegistry.h"
#include "engine/ai/inference/Model.h"
#include "engine/core/Logger.h"

#include <algorithm>
#include <sstream>

namespace liz {

// -- Registration ---------------------------------------------------------------
bool ModelRegistry::register_model(std::string name,
                                    ModelType type,
                                    std::string version) {
    // Duplicate check.
    for (const auto& e : entries_) {
        if (e.name == name) {
            std::ostringstream oss;
            oss << "ModelRegistry: model '" << name << "' already registered";
            LIZ_WARN(oss.str());
            return false;
        }
    }

    entries_.push_back({std::move(name), type, std::move(version)});

    std::ostringstream oss;
    oss << "ModelRegistry: registered model '"
        << entries_.back().name << "' ["
        << model_type_to_string(entries_.back().type) << "] v"
        << entries_.back().version;
    LIZ_INFO(oss.str());

    return true;
}

// -- Query ---------------------------------------------------------------------
bool ModelRegistry::has_model(std::string_view name) const {
    for (const auto& e : entries_) {
        if (e.name == name) return true;
    }
    return false;
}

std::vector<std::string> ModelRegistry::get_model_list() const {
    std::vector<std::string> names;
    names.reserve(entries_.size());
    for (const auto& e : entries_) {
        names.push_back(e.name);
    }
    return names;
}

std::vector<std::string> ModelRegistry::get_models_by_type(ModelType type) const {
    std::vector<std::string> names;
    for (const auto& e : entries_) {
        if (e.type == type) {
            names.push_back(e.name);
        }
    }
    return names;
}

std::size_t ModelRegistry::size() const {
    return entries_.size();
}

// -- Logging -------------------------------------------------------------------
void ModelRegistry::log_all() const {
    LIZ_INFO("ModelRegistry: all registered models:");
    for (const auto& e : entries_) {
        std::ostringstream oss;
        oss << "  - '" << e.name << "' ["
            << model_type_to_string(e.type) << "] v" << e.version;
        LIZ_INFO(oss.str());
    }
}

} // namespace liz