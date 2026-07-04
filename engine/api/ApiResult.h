#pragma once

namespace liz {

/// Result codes returned by API operations.
///
/// Every public API method returns an ApiResult so callers can handle
/// errors without relying on exceptions.
enum class ApiResult {
    Success,          ///< Operation completed successfully
    Warning,          ///< Operation succeeded with caveats
    Failed,           ///< General failure
    InvalidArgument,  ///< One or more arguments were invalid
    NotFound,         ///< Requested resource does not exist
    AlreadyExists,    ///< Resource already exists (cannot duplicate)
    Busy,             ///< Engine is busy and cannot accept the request
    Unsupported,      ///< Operation is not supported in this configuration
    InternalError,   ///< An internal engine error occurred
    Timeout           ///< Operation exceeded the time limit
};

/// Convert an ApiResult to a human-readable string.
inline const char* api_result_to_string(ApiResult result) {
    switch (result) {
        case ApiResult::Success:         return "Success";
        case ApiResult::Warning:         return "Warning";
        case ApiResult::Failed:          return "Failed";
        case ApiResult::InvalidArgument: return "InvalidArgument";
        case ApiResult::NotFound:        return "NotFound";
        case ApiResult::AlreadyExists:   return "AlreadyExists";
        case ApiResult::Busy:            return "Busy";
        case ApiResult::Unsupported:     return "Unsupported";
        case ApiResult::InternalError:   return "InternalError";
        case ApiResult::Timeout:         return "Timeout";
    }
    return "Unknown";
}

} // namespace liz
