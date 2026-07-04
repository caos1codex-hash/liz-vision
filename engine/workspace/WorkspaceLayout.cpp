#include "engine/workspace/WorkspaceLayout.h"

#include <sstream>

namespace liz {

std::string WorkspaceLayout::to_string() const {
    std::ostringstream oss;
    oss << "Layout{"
        << " panels=" << panels_.size()
        << " active=" << active_panel_
        << " pos=" << position_x_ << "x" << position_y_
        << " size=" << width_ << "x" << height_
        << " visible=" << (visible_ ? "yes" : "no")
        << " }";
    return oss.str();
}

} // namespace liz
