#include "engine/workspace/WorkspacePreferences.h"

#include <sstream>

namespace liz {

std::string WorkspacePreferences::to_string() const {
    std::ostringstream oss;
    oss << "Preferences{"
        << " theme=" << theme_
        << " lang=" << language_
        << " autosave=" << (autosave_enabled_ ? "on" : "off")
        << " gpu=" << (gpu_enabled_ ? "on" : "off")
        << " diag=" << (diagnostics_enabled_ ? "on" : "off")
        << " auto_validate=" << (auto_validate_pipeline_ ? "on" : "off")
        << " threads=" << thread_count_
        << " batch=" << batch_size_
        << " recent=" << recent_projects_limit_
        << " }";
    return oss.str();
}

} // namespace liz
