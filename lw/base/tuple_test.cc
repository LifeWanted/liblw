#include "lw/base/tuple.h"

#include <tuple>
#include <type_traits>

#include "gtest/gtest.h"

namespace lw {
namespace {

TEST(IsTupleMember, TrueForMembers) {
  EXPECT_TRUE((is_tuple_member_v<char, std::tuple<char>>));
  EXPECT_TRUE((is_tuple_member_v<char, std::tuple<int, char>>));
  EXPECT_TRUE((is_tuple_member_v<char, std::tuple<int, char, double>>));
}

TEST(IsTupleMember, FalseForNonMembers) {
  EXPECT_FALSE((is_tuple_member_v<float, std::tuple<char>>));
  EXPECT_FALSE((is_tuple_member_v<float, std::tuple<int, char>>));
  EXPECT_FALSE((is_tuple_member_v<float, std::tuple<int, char, double>>));
}

TEST(FlatTuple, RemovesNestedTuples) {
  EXPECT_TRUE((
    std::is_same_v<
      flat_tuple_t<std::tuple<char, std::tuple<int>>>,
      std::tuple<char, int>
    >
  ));
}

TEST(FlatTuple, RecursivelyRemovesTuples) {
  EXPECT_TRUE((
    std::is_same_v<
      flat_tuple_t<std::tuple<char, std::tuple<std::tuple<int>>>>,
      std::tuple<char, int>
    >
  ));
}

TEST(FlatTuple, DoesNotDeduplicate) {
  EXPECT_TRUE((
    std::is_same_v<
      flat_tuple_t<std::tuple<char, std::tuple<char>>>,
      std::tuple<char, char>
    >
  ));
}

TEST(DeduplicateTuple, Deduplicates) {
  EXPECT_TRUE((
    std::is_same_v<
      deduplicate_tuple_t<std::tuple<char, char>>,
      std::tuple<char>
    >
  ));
  EXPECT_TRUE((
    std::is_same_v<
      deduplicate_tuple_t<std::tuple<char, char, int>>,
      std::tuple<char, int>
    >
  ));
  EXPECT_TRUE((
    std::is_same_v<
      deduplicate_tuple_t<std::tuple<char, int, char>>,
      std::tuple<char, int>
    >
  ));
  EXPECT_TRUE((
    std::is_same_v<
      deduplicate_tuple_t<std::tuple<int, char, char, int>>,
      std::tuple<int, char>
    >
  ));
}

TEST(TupleApplyTypeModification, AppliesModificationToEveryMember) {
  EXPECT_TRUE((
    std::is_same_v<
      apply_type_modification_t<
        std::remove_cvref,
        std::tuple<const int, char&, volatile float>
      >,
      std::tuple<int, char, float>
    >
  ));
}

TEST(SanitizeTuple, FlattensDeduplicatesAndDecaysTupleTypes) {
  EXPECT_TRUE((
    std::is_same_v<
      sanitize_tuple_t<
        std::tuple<
          const int,
          std::tuple<char, float&>,
          std::tuple<const float&, std::tuple<const char, volatile int>>
        >
      >,
      std::tuple<int, char, float>
    >
  ));
}

}
}
