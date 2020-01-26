#include "lw/err/error.h"

#include <experimental/source_location>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace lw {
namespace {

using testing::MatchesRegex;

class TestError : public Error {
public:
  TestError(
    const std::experimental::source_location& loc =
      std::experimental::source_location::current()
  ):
    Error("TestError", loc)
  {}
};

TEST(ErrorTest, StreamingMessage) {
  TestError err;
  err << "Foo bar";
  EXPECT_THAT(err.what(), MatchesRegex(".*TestError.*Foo bar.*"));
}

TEST(ErrorTest, CatchAsRuntimeError) {
  try {
    throw TestError() << "oh noes!";
  } catch (const std::runtime_error& err) {
    EXPECT_THAT(err.what(), MatchesRegex(".*TestError.*oh noes!.*"));
  } catch (...) {
    FAIL() << "Caught in generic-catch-all block.";
  }
}

TEST(ErrorTest, CatchAsExactErrorType) {
  try {
    throw TestError() << "oh noes!";
  } catch (const TestError& err) {
    EXPECT_THAT(err.what(), MatchesRegex(".*TestError.*oh noes!.*"));
  } catch (...) {
    FAIL() << "Caught in generic-catch-all block.";
  }
}

TEST(ErrorTest, SourceLocationCaptured) {
  TestError err;
  const int line = __LINE__ - 1; // Previous line.
  EXPECT_EQ(err.where().file_name(), __FILE__);
  EXPECT_EQ(err.where().line(), line);
}

}
}
