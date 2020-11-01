#include "lw/log/log.h"

#include <experimental/source_location>
#include <sstream>
#include <ostream>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lw/flags/flags.h"

// Defined in log.cpp
LW_DECLARE_FLAG(bool, enable_trace_logs);
LW_DECLARE_FLAG(bool, enable_debug_logs);

namespace lw {
namespace {

using ::testing::MatchesRegex;

class TestLogSink: public LogSink {
public:
  explicit TestLogSink(std::ostream& stream): ostream{stream} {}

  std::ostream& start_log(
    LogLevel level,
    std::experimental::source_location loc
  ) override {
    return ostream
      << level_name(level).at(0) << ':' << loc.file_name() << ':' << loc.line()
      << ": ";
  }

  std::ostream& ostream;
};

class TestLog: public ::testing::Test {
protected:
  TestLog() {
    Logger::instance().sink(TestLogSink{ostream});
  }

  std::stringstream ostream;
};

TEST_F(TestLog, SimpleLog) {
  log(INFO) << "foo";
  EXPECT_THAT(
    ostream.str(),
    MatchesRegex("I:lw/log/log_test.cpp:[0-9]+: foo\n")
  );
}

TEST_F(TestLog, DisabledDebug) {
  flags::enable_debug_logs = false;
  log(DEBUG) << "This won't print.";
  EXPECT_TRUE(ostream.str().empty());
}

}
}
