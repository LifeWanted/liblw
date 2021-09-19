#include "lw/err/macros.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lw/err/canonical.h"

namespace lw {
namespace {

using testing::MatchesRegex;

TEST(ErrorMacros, NullCheckThrowsOnNull) {
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

TEST(ErrorMacros, InternalNullCheckThrowsOnNull) {
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

TEST(ErrorMacros, CheckGreaterThan) {
  try {
    int small = 1;
    int big = 42;
    LW_CHECK_GT(big, small) << "Does not throw.";
    LW_CHECK_GT(small, big) << "Does throw.";
  } catch (const FailedPrecondition& err) {
    EXPECT_THAT(
      err.what(),
      MatchesRegex(".*`small` \\(1\\) > `big` \\(42\\).*Does throw.")
    );
  } catch (...) {
    FAIL() << "Should throw FailedPrecondition.";
  }
}

TEST(ErrorMacros, CheckGreaterThanTyped) {
  try {
    int small = 1;
    int big = 42;
    LW_CHECK_GT_TYPED(big, small, Internal) << "Does not throw.";
    LW_CHECK_GT_TYPED(small, big, Internal) << "Does throw.";
  } catch (const Internal& err) {
    EXPECT_THAT(
      err.what(),
      MatchesRegex(".*`small` \\(1\\) > `big` \\(42\\).*Does throw.")
    );
  } catch (...) {
    FAIL() << "Should throw Internal.";
  }
}

TEST(ErrorMacros, CheckGreaterThanOrEqualTo) {
  try {
    int small = 1;
    int big = 42;
    LW_CHECK_GTE(big, small) << "Does not throw.";
    LW_CHECK_GTE(big, big) << "Does not throw.";
    LW_CHECK_GTE(small, big) << "Does throw.";
  } catch (const FailedPrecondition& err) {
    EXPECT_THAT(
      err.what(),
      MatchesRegex(".*`small` \\(1\\) >= `big` \\(42\\).*Does throw.")
    );
  } catch (...) {
    FAIL() << "Should throw FailedPrecondition.";
  }
}

TEST(ErrorMacros, CheckGreaterThanOrEqualToTyped) {
  try {
    int small = 1;
    int big = 42;
    LW_CHECK_GTE_TYPED(big, small, Internal) << "Does not throw.";
    LW_CHECK_GTE_TYPED(big, big, Internal) << "Does not throw.";
    LW_CHECK_GTE_TYPED(small, big, Internal) << "Does throw.";
  } catch (const Internal& err) {
    EXPECT_THAT(
      err.what(),
      MatchesRegex(".*`small` \\(1\\) >= `big` \\(42\\).*Does throw.")
    );
  } catch (...) {
    FAIL() << "Should throw Internal.";
  }
}

}
}
