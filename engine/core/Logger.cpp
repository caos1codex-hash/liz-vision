#include "engine/core/Logger.h"

#include <cstdio>
#include <ctime>

namespace liz {

// ── Singleton access ─────────────────────────────────────────────────────────
Logger& Logger::instance() {
    static Logger logger;
    return logger;
}

// ── Level configuration ──────────────────────────────────────────────────────
void Logger::set_level(LogLevel level) {
    min_level_ = level;
}

// ── Core logging ─────────────────────────────────────────────────────────────
void Logger::log(LogLevel level,
                 std::string_view /*file*/,
                 int /*line*/,
                 std::string_view message) {
    if (level < min_level_) {
        return;
    }

    // Simple timestamp
    std::timespec ts{};
    std::timespec_get(&ts, TIME_UTC);
    std::tm tm_buf{};
    localtime_r(&ts.tv_sec, &tm_buf);

    char time_str[32];
    std::strftime(time_str, sizeof(time_str), "%H:%M:%S", &tm_buf);

    // Color the tag based on severity (ANSI escape codes)
    const char* color = "\033[0m";
    switch (level) {
        case LogLevel::Trace: color = "\033[90m";  break; // gray
        case LogLevel::Debug: color = "\033[36m";  break; // cyan
        case LogLevel::Info:  color = "\033[32m";  break; // green
        case LogLevel::Warn:  color = "\033[33m";  break; // yellow
        case LogLevel::Error: color = "\033[31m";  break; // red
        case LogLevel::Fatal: color = "\033[1;31m";break; // bold red
    }
    const char* reset = "\033[0m";

    std::fprintf(stdout, "%s[%s] [%s%-5s%s] %s\n",
                 reset, time_str, color, level_tag(level), reset,
                 message.data());
    std::fflush(stdout);
}

// ── Helpers ──────────────────────────────────────────────────────────────────
const char* Logger::level_tag(LogLevel level) {
    switch (level) {
        case LogLevel::Trace: return "TRACE";
        case LogLevel::Debug: return "DEBUG";
        case LogLevel::Info:  return "INFO";
        case LogLevel::Warn:  return "WARN";
        case LogLevel::Error: return "ERROR";
        case LogLevel::Fatal: return "FATAL";
    }
    return "?????";
}

} // namespace liz