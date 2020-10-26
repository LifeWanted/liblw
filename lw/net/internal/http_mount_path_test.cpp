#include "lw/net/internal/http_mount_path.h"

#include "gtest/gtest.h"

namespace lw::net::internal {

TEST(MountPath, BasicEndpoint) {
  MountPath mp = MountPath::parse_endpoint("/foo");
  auto result = mp.match("/foo");
  EXPECT_TRUE(result.has_value());
  EXPECT_TRUE(result->empty());

  EXPECT_FALSE(mp.match("/bar"));
}

TEST(MountPath, TwoPartEndpoint) {
  MountPath mp = MountPath::parse_endpoint("/foo/bar");
  auto result = mp.match("/foo/bar");
  ASSERT_TRUE(result.has_value());
  EXPECT_TRUE(result->empty());

  EXPECT_FALSE(mp.match("/bar/foo"));
}

TEST(MountPath, SimpleParameterCapture) {
  MountPath mp = MountPath::parse_endpoint("/:foo");
  auto result = mp.match("/fizz");
  ASSERT_TRUE(result.has_value());
  ASSERT_TRUE(result->contains("foo"));
  EXPECT_EQ(result->at("foo"), "fizz");

  EXPECT_FALSE(mp.match("/flim/flam"));
}

TEST(MountPath, DoubleParameterCapture) {
  MountPath mp = MountPath::parse_endpoint("/:foo/:bar");
  auto result = mp.match("/flim/flam");
  ASSERT_TRUE(result.has_value());
  ASSERT_TRUE(result->contains("foo"));
  ASSERT_TRUE(result->contains("bar"));
  EXPECT_EQ(result->at("foo"), "flim");
  EXPECT_EQ(result->at("bar"), "flam");

  EXPECT_FALSE(mp.match("/onepart"));
  EXPECT_FALSE(mp.match("/three/parts/here"));
}

TEST(MountPath, MixedCaptureNoCapture) {
  MountPath mp = MountPath::parse_endpoint("/foo/:foo/bar/:bar/baz");
  auto result = mp.match("/foo/capture/bar/this/baz");
  ASSERT_TRUE(result.has_value());
  ASSERT_TRUE(result->contains("foo"));
  ASSERT_TRUE(result->contains("bar"));
  EXPECT_EQ(result->at("foo"), "capture");
  EXPECT_EQ(result->at("bar"), "this");
  EXPECT_EQ(result->size(), 2);
}

TEST(MountPath, IntValidatedParameterCapture) {
  MountPath mp = MountPath::parse_endpoint("/:[int]foo");
  auto result = mp.match("/1234");
  ASSERT_TRUE(result.has_value());
  ASSERT_TRUE(result->contains("foo"));
  EXPECT_EQ(result->at("foo"), "1234");

  result = mp.match("/-4321");
  ASSERT_TRUE(result.has_value());
  ASSERT_TRUE(result->contains("foo"));
  EXPECT_EQ(result->at("foo"), "-4321");

  EXPECT_FALSE(mp.match("/foobar"));
  EXPECT_FALSE(mp.match("/123foo"));
  EXPECT_FALSE(mp.match("/foo123"));
}

}
