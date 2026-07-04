#include "engine/ai/inference/Model.h"
#include "engine/core/Logger.h"

#include <sstream>

namespace liz {

// -- Enum converter --------------------------------------------------------------
const char* model_type_to_string(ModelType type) {
    switch (type) {
        case ModelType::Upscaler:     return "Upscaler";
        case ModelType::Interpolator: return "Interpolator";
        case ModelType::Denoiser:     return "Denoiser";
    }
    return "Unknown";
}

// -- Utility --------------------------------------------------------------------
std::string Model::summary() const {
    std::ostringstream oss;
    oss << "Model{"
        << "name='" << name() << "'"
        << " version='" << version() << "'"
        << " type=" << model_type_to_string(type())
        << " in=" << input_format().name
        << " out=" << output_format().name
        << "}";
    return oss.str();
}

} // namespace liz