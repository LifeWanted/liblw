#include "lw/base/math.h"

#include "gtest/gtest.h"

namespace lw {
namespace {

TEST(LerpInt, LowEnd) {
  EXPECT_EQ(lerp(3, 7, 0.0), 3);
}

TEST(LerpInt, Middle) {
  EXPECT_EQ(lerp(3, 7, 0.5), 5);
}

TEST(LerpInt, HighEnd) {
  EXPECT_EQ(lerp(3, 7, 1.0), 7);
}

TEST(LerpFloat, LowEnd) {
  EXPECT_EQ(lerp(3.0, 7.0, 0.0), 3.0);
}

TEST(LerpFloat, Middle) {
  EXPECT_EQ(lerp(3.0, 7.0, 0.5), 5.0);
}

TEST(LerpFloat, HighEnd) {
  EXPECT_EQ(lerp(3.0, 7.0, 1.0), 7.0);
}

}
}
