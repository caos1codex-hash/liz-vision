#include "engine/project/ProjectMetadata.h"

#include <sstream>

namespace liz {

std::string ProjectMetadata::to_string() const {
    std::ostringstream oss;
    oss << "Metadata{";
    if (!company_.empty())       oss << " company=" << company_;
    if (!website_.empty())       oss << " website=" << website_;
    if (!author_.empty())        oss << " author=" << author_;
    if (!version_.empty())       oss << " version=" << version_;
    if (!description_.empty())   oss << " desc=" << description_;
    if (!tags_.empty()) {
        oss << " tags=[";
        for (std::size_t i = 0; i < tags_.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << tags_[i];
        }
        oss << "]";
    }
    oss << " }";
    return oss.str();
}

} // namespace liz
