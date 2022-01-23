#include "lw/err/error.h"

#include <experimental/source_location>
#include <string_view>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace lw {
namespace {

using ::std::experimental::source_location;
using ::testing::ContainerEq;
using ::testing::HasSubstr;
using ::testing::MatchesRegex;
using ::testing::Not;

class TestError : public Error {
public:
  TestError(const source_location& loc = source_location::current()):
    Error("TestError", loc)
  {}
};

bool equal(const source_location& lhs, const source_location& rhs) {
  if (lhs.column() != rhs.column()) return false;
  if (lhs.line() != rhs.line()) return false;
  std::string_view lhs_file = lhs.file_name();
  std::string_view rhs_file = rhs.file_name();
  return lhs_file == rhs_file;
}

TEST(Error, StreamingMessage) {
  TestError err;
  err << "Foo bar";
  EXPECT_THAT(err.what(), MatchesRegex(".*TestError.*Foo bar.*"));
}

TEST(Error, CatchAsRuntimeError) {
  try {
    throw TestError() << "oh noes!";
  } catch (const std::runtime_error& err) {
    EXPECT_THAT(err.what(), MatchesRegex(".*TestError.*oh noes!.*"));
  } catch (...) {
    FAIL() << "Caught in generic-catch-all block.";
  }
}

TEST(Error, CatchAsExactErrorType) {
  try {
    throw TestError() << "oh noes!";
  } catch (const TestError& err) {
    EXPECT_THAT(err.what(), MatchesRegex(".*TestError.*oh noes!.*"));
  } catch (...) {
    FAIL() << "Caught in generic-catch-all block.";
  }
}

TEST(Error, SourceLocationCaptured) {
  TestError err;
  const int line = __LINE__ - 1; // Previous line.
  EXPECT_EQ(err.where().file_name(), __FILE__);
  EXPECT_EQ(err.where().line(), line);
}

TEST(Error, Wrap) {
  TestError err1;
  err1 << "Earlier error.";
  TestError err2 = wrap<TestError>(err1) << "Later error.";

  EXPECT_THAT(err2.what(), HasSubstr("Later error."));
  EXPECT_THAT(err2.what(), Not(HasSubstr("Earlier error.")));

  ErrorStack::Iterator itr = err2.stack().begin();
  ASSERT_NE(itr, err2.stack().end());
  EXPECT_TRUE(equal(itr->location(), err2.where()));
  EXPECT_EQ(itr->message(), err2.what());

  ASSERT_NE(++itr, err2.stack().end());
  EXPECT_TRUE(equal(itr->location(), err1.where()));
  EXPECT_EQ(itr->message(), err1.what());

  EXPECT_EQ(++itr, err2.stack().end());
}

}
}
