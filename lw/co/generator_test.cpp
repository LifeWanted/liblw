#include "lw/co/generator.h"

#include "gtest/gtest.h"

namespace lw::co {
namespace {

Generator<int> yield_some() {
  co_yield 1;
  co_yield 2;
}

TEST(Generator, BasicGeneration) {
  auto gen = yield_some();
  int expected = 1;
  while (gen.next()) {
    EXPECT_EQ(gen.value(), expected);
    ++expected;
  }
  EXPECT_EQ(expected, 3);
}

}
}
