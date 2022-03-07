#include "lw/cli/color.h"

#include "gtest/gtest.h"

namespace lw::cli {
namespace {

constexpr std::uint32_t TEST_COLOR = 0x01020304;

TEST(Color, DefaultBlack) {
  Color color;
  EXPECT_EQ(color.r, 0x00);
  EXPECT_EQ(color.g, 0x00);
  EXPECT_EQ(color.b, 0x00);
  EXPECT_EQ(color.a, 0xff);
}

TEST(Color, UInt32IsRGB) {
  Color color{TEST_COLOR};
  EXPECT_EQ(color.r, 0x02);
  EXPECT_EQ(color.g, 0x03);
  EXPECT_EQ(color.b, 0x04);
  EXPECT_EQ(color.a, 0xff);
}

TEST(Color, UInt32WithRGBTag) {
  Color color{rgb, TEST_COLOR};
  EXPECT_EQ(color.r, 0x02);
  EXPECT_EQ(color.g, 0x03);
  EXPECT_EQ(color.b, 0x04);
  EXPECT_EQ(color.a, 0xff);
}

TEST(Color, UInt32WithBGRTag) {
  Color color{bgr, TEST_COLOR};
  EXPECT_EQ(color.r, 0x04);
  EXPECT_EQ(color.g, 0x03);
  EXPECT_EQ(color.b, 0x02);
  EXPECT_EQ(color.a, 0xff);
}

TEST(Color, UInt32WithARGBTag) {
  Color color{argb, TEST_COLOR};
  EXPECT_EQ(color.r, 0x02);
  EXPECT_EQ(color.g, 0x03);
  EXPECT_EQ(color.b, 0x04);
  EXPECT_EQ(color.a, 0x01);
}

TEST(Color, UInt32WithRGBATag) {
  Color color{rgba, TEST_COLOR};
  EXPECT_EQ(color.r, 0x01);
  EXPECT_EQ(color.g, 0x02);
  EXPECT_EQ(color.b, 0x03);
  EXPECT_EQ(color.a, 0x04);
}

TEST(Color, PiecewiseIsRGB) {
  Color color{0x01, 0x02, 0x03};
  EXPECT_EQ(color.r, 0x01);
  EXPECT_EQ(color.g, 0x02);
  EXPECT_EQ(color.b, 0x03);
  EXPECT_EQ(color.a, 0xff);
}

TEST(Color, PiecewiseIsARGB) {
  Color color{0x01, 0x02, 0x03, 0x04};
  EXPECT_EQ(color.r, 0x02);
  EXPECT_EQ(color.g, 0x03);
  EXPECT_EQ(color.b, 0x04);
  EXPECT_EQ(color.a, 0x01);
}

TEST(Color, PiecewiseWithRGBTag) {
  Color color{rgb, 0x01, 0x02, 0x03};
  EXPECT_EQ(color.r, 0x01);
  EXPECT_EQ(color.g, 0x02);
  EXPECT_EQ(color.b, 0x03);
  EXPECT_EQ(color.a, 0xff);
}

TEST(Color, PiecewiseWithBGRTag) {
  Color color{bgr, 0x01, 0x02, 0x03};
  EXPECT_EQ(color.r, 0x03);
  EXPECT_EQ(color.g, 0x02);
  EXPECT_EQ(color.b, 0x01);
  EXPECT_EQ(color.a, 0xff);
}

TEST(Color, PiecewiseWithARGBTag) {
  Color color{argb, 0x01, 0x02, 0x03, 0x04};
  EXPECT_EQ(color.r, 0x02);
  EXPECT_EQ(color.g, 0x03);
  EXPECT_EQ(color.b, 0x04);
  EXPECT_EQ(color.a, 0x01);
}

TEST(Color, PiecewiseWithRGBATag) {
  Color color{rgba, 0x01, 0x02, 0x03, 0x04};
  EXPECT_EQ(color.r, 0x01);
  EXPECT_EQ(color.g, 0x02);
  EXPECT_EQ(color.b, 0x03);
  EXPECT_EQ(color.a, 0x04);
}

TEST(Color, Equality) {
  Color a{0x000001};
  Color b{0x000001};
  Color c{0x010001};
  EXPECT_TRUE(a == b); // Explicitly testing `==` operator.
  EXPECT_EQ(b, a);
  EXPECT_FALSE(a == c);
  EXPECT_FALSE(c == a);
}

TEST(Color, Inequality) {
  Color a{0x000001};
  Color b{0x000001};
  Color c{0x010001};
  EXPECT_TRUE(a != c); // Explicitly testing `!=` operator.
  EXPECT_NE(c, a);
  EXPECT_FALSE(a != b);
  EXPECT_FALSE(b != a);
}

}
}
