#include "lw/err/macros.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lw/err/canonical.h"

namespace lw {
namespace {

using testing::MatchesRegex;

TEST(ErrMacros, NullCheckThrowsOnNull) {
  try {
    void* foo = nullptr;
    void* bar = &foo;
    LW_CHECK_NULL(bar); // Does not throw.
    LW_CHECK_NULL(foo); // Does throw.
  } catch (const InvalidArgument& err) {
    EXPECT_THAT(
      err.what(), MatchesRegex(".*foo must not be null.*")
    );
  } catch (...) {
    FAIL() << "Should throw InvalidArgument.";
  }
}

TEST(ErrMacros, InternalNullCheckThrowsOnNull) {
  try {
    void* foo = nullptr;
    void* bar = &foo;
    LW_CHECK_NULL_INTERNAL(bar) << "Does not throw.";
    LW_CHECK_NULL_INTERNAL(foo) << "Does throw.";
  } catch (const Internal& err) {
    EXPECT_THAT(err.what(), MatchesRegex(".*Does throw."));
  } catch (...) {
    FAIL() << "Should throw Internal.";
  }
}

}
}
