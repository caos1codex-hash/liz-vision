#pragma once

#include <mutex>
#include <string>
#include <string_view>

namespace liz {

/// Log severity levels.
enum class LogLevel {
    Trace,
    Debug,
    Info,
    Warn,
    Error,
    Fatal
};

/// Singleton logger that writes formatted messages to the console.
/// Thread-safe: uses a mutex to serialize output from multiple threads.
class Logger {
public:
    /// Retrieve the global Logger instance.
    static Logger& instance();

    /// Set the minimum severity that will actually be printed.
    void set_level(LogLevel level);

    /// Log a message with the given severity.
    /// File and line are captured automatically via the LIZ_LOG macro.
    void log(LogLevel level,
             std::string_view file,
             int line,
             std::string_view message);

private:
    Logger() = default;

    /// Convert a LogLevel to a short tag string (e.g. "INFO").
    static const char* level_tag(LogLevel level);

    LogLevel      min_level_ = LogLevel::Info;
    mutable std::mutex mutex_;
};

} // namespace liz

// ── Convenience macros ───────────────────────────────────────────────────────
#define LIZ_LOG(level, msg) \
    ::liz::Logger::instance().log(level, __FILE__, __LINE__, msg)

#define LIZ_TRACE(msg) LIZ_LOG(::liz::LogLevel::Trace, msg)
#define LIZ_DEBUG(msg) LIZ_LOG(::liz::LogLevel::Debug, msg)
#define LIZ_INFO(msg)  LIZ_LOG(::liz::LogLevel::Info,  msg)
#define LIZ_WARN(msg)  LIZ_LOG(::liz::LogLevel::Warn,  msg)
#define LIZ_ERROR(msg) LIZ_LOG(::liz::LogLevel::Error, msg)
#define LIZ_FATAL(msg) LIZ_LOG(::liz::LogLevel::Fatal, msg)