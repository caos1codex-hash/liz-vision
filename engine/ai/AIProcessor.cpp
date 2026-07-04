#include "engine/ai/AIProcessor.h"

namespace liz {

const char* ai_processor_type_to_string(AIProcessorType type) {
    switch (type) {
        case AIProcessorType::Upscaler:     return "Upscaler";
        case AIProcessorType::Interpolator: return "Interpolator";
        case AIProcessorType::Denoiser:     return "Denoiser";
        case AIProcessorType::Enhancer:     return "Enhancer";
        case AIProcessorType::Custom:       return "Custom";
    }
    return "Unknown";
}

} // namespace liz