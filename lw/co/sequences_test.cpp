#include "lw/co/sequences.h"

#include <utility>
#include <vector>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

namespace lw::co {
namespace {

using ::testing::ElementsAre;

TEST(Range, ForwardIncrement) {
  std::vector<int> values;
  auto gen = range(0, 10);
  while (gen.next()) {
    values.push_back(gen.value());
  }
  EXPECT_THAT(values, ElementsAre(0, 1, 2, 3, 4, 5, 6, 7, 8, 9));
}

TEST(Range, ForwardLargeIncrement) {
  std::vector<int> values;
  auto gen = range(0, 10, 3);
  while (gen.next()) {
    values.push_back(gen.value());
  }
  EXPECT_THAT(values, ElementsAre(0, 3, 6, 9));
}

TEST(Range, ForwardObjectRange) {
  std::vector<std::pair<int, float>> values{
    {0, 1.1f}, {1, 2.2f}, {2, 3.3f},
    {3, 4.4f}, {4, 5.5f}, {5, 6.6f},
    {6, 7.7f}, {7, 8.8f}, {8, 9.9f},
  };
  auto gen = range(values.begin(), values.end());
  int i = 0;
  for (; gen.next(); ++i) {
    EXPECT_EQ(gen.value()->first, i);
  }
  EXPECT_EQ(i, values.size());
}

TEST(Range, ReverseIncrement) {
  std::vector<int> values;
  auto gen = range(10, 0);
  while (gen.next()) {
    values.push_back(gen.value());
  }
  EXPECT_THAT(values, ElementsAre(10, 9, 8, 7, 6, 5, 4, 3, 2, 1));
}

TEST(Range, ReverseLargeIncrement) {
  std::vector<int> values;
  auto gen = range(10, 0, 3);
  while (gen.next()) {
    values.push_back(gen.value());
  }
  EXPECT_THAT(values, ElementsAre(10, 7, 4, 1));
}

}
}
