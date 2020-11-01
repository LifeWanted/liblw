#pragma once

#include <experimental/source_location>
#include <memory>
#include <ostream>
#include <string_view>

namespace lw {

enum LogLevel {
  TRACE   = 0,
  DEBUG   = 1,
  INFO    = 2,
  WARN    = 3,
  ERROR   = 4
};

class LogSink {
public:
  virtual ~LogSink() = default;

  virtual std::ostream& start_log(
    LogLevel level,
    std::experimental::source_location loc
  ) = 0;

protected:
  std::string_view level_name(LogLevel level) const;
};

class Logger {
public:
  static Logger& instance();

  LogSink& sink();

  template <typename LogSinkType>
  void sink(LogSinkType&& sink) {
    _sink = std::make_unique<LogSinkType>(std::forward<LogSinkType>(sink));
  }
private:
  std::unique_ptr<LogSink> _sink;
};

class LogWriter {
public:
  explicit LogWriter(std::ostream* stream): _stream{stream} {}
  ~LogWriter() {
    if (_stream) (*_stream) << std::endl;
  }

  template <typename T>
  LogWriter& operator<<(T&& value) {
    if (_stream) (*_stream) << std::forward<T>(value);
    return *this;
  }

private:
  std::ostream* _stream;
};

LogWriter log(
  LogLevel level,
  std::experimental::source_location loc =
    std::experimental::source_location::current()
);

}
