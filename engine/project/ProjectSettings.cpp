#include "engine/project/ProjectSettings.h"

#include <sstream>

namespace liz {

std::string ProjectSettings::to_string() const {
    std::ostringstream oss;
    oss << "Settings{"
        << " gpu=" << (gpu_enabled_ ? "on" : "off")
        << " diag=" << (diagnostics_enabled_ ? "on" : "off")
        << " auto_validate=" << (pipeline_auto_validation_ ? "on" : "off")
        << " autosave=" << (autosave_enabled_ ? "on" : "off")
        << " threads=" << thread_count_
        << " lang=" << language_
        << " theme=" << theme_
        << " batch=" << batch_size_
        << " }";
    return oss.str();
}

} // namespace liz
