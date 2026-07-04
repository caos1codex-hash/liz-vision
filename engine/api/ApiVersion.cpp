#include "engine/api/ApiVersion.h"

#include <sstream>

namespace liz {

std::string ApiVersion::version_string() const {
    std::ostringstream oss;
    oss << major_ << "." << minor_ << "." << patch_;
    return oss.str();
}

std::string ApiVersion::full_version_string() const {
    std::ostringstream oss;
    oss << major_ << "." << minor_ << "." << patch_ << "+" << build_;
    return oss.str();
}

bool ApiVersion::compatible_with(const ApiVersion& other) const {
    return major_ == other.major_;
}

bool ApiVersion::operator==(const ApiVersion& other) const {
    return major_ == other.major_ &&
           minor_ == other.minor_ &&
           patch_ == other.patch_ &&
           build_ == other.build_;
}

bool ApiVersion::operator!=(const ApiVersion& other) const {
    return !(*this == other);
}

bool ApiVersion::operator<(const ApiVersion& other) const {
    if (major_ != other.major_) return major_ < other.major_;
    if (minor_ != other.minor_) return minor_ < other.minor_;
    if (patch_ != other.patch_) return patch_ < other.patch_;
    return build_ < other.build_;
}

} // namespace liz
