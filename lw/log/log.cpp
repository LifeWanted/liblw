#include "lw/log/log.h"

#include <experimental/source_location>
#include <iostream>
#include <ostream>
#include <string_view>

#include "lw/flags/flags.h"

LW_FLAG(
  bool, enable_trace_logs, false,
  "Controls whether log(TRACE) calls capture output."
);
LW_FLAG(
  bool, enable_debug_logs, false,
  "Controls whether log(TRACE) and log(DEBUG) calls capture output."
);
LW_FLAG(
  bool, enable_logs, true,
  "Controls whether any log messages are captured."
);

namespace lw {
namespace {

class DefaultLogSink: public LogSink {
public:
  std::ostream& start_log(
    LogLevel level,
    std::experimental::source_location loc
  ) override {
    return std::cout
      << level_name(level).at(0) << ':' << loc.file_name() << ':' << loc.line()
      << ": ";
  }
};

}

std::string_view LogSink::level_name(LogLevel level) const {
  switch (level) {
    case TRACE: return "Trace";
    case DEBUG: return "Debug";
    case INFO:  return "Info";
    case WARN:  return "Warn";
    case ERROR: return "Error";
    default:    return "Unknown";
  }
}

Logger& Logger::instance() {
  static Logger logger;
  return logger;
}

LogSink& Logger::sink() {
  static DefaultLogSink default_sink;
  return _sink ? *_sink : default_sink;
}

LogWriter log(LogLevel level, std::experimental::source_location loc) {
  if (!flags::enable_logs) return LogWriter{nullptr};
  if (level == DEBUG && !flags::enable_debug_logs) return LogWriter{nullptr};
  if (
    level == TRACE &&
    (!flags::enable_trace_logs || !flags::enable_debug_logs)
  ) {
    return LogWriter{nullptr};
  }
  return LogWriter{&Logger::instance().sink().start_log(level, loc)};
}

}
