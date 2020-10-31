#include "lw/http/internal/http_mount_path.h"

#include "gtest/gtest.h"
#include "lw/http/http_handler.h"

namespace lw::http::internal {
namespace {

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

TEST(MountPath, UIntValidatedParameterCapture) {
  MountPath mp = MountPath::parse_endpoint("/:[uint]foo");
  auto result = mp.match("/1234");
  ASSERT_TRUE(result.has_value());
  ASSERT_TRUE(result->contains("foo"));
  EXPECT_EQ(result->at("foo"), "1234");

  EXPECT_FALSE(mp.match("/-4321"));
  EXPECT_FALSE(mp.match("/foobar"));
  EXPECT_FALSE(mp.match("/123foo"));
  EXPECT_FALSE(mp.match("/foo123"));
}

TEST(MountPath, RegexParameterCapture) {
  MountPath mp = MountPath::parse_endpoint("/:[re]f[o]{2,}");
  auto result = mp.match("/foo");
  ASSERT_TRUE(result.has_value());
  ASSERT_TRUE(result->contains("1"));
  EXPECT_EQ(result->at("1"), "foo");

  EXPECT_TRUE(mp.match("/fooo"));
  EXPECT_TRUE(mp.match("/fooooooooooooooooooo"));
  EXPECT_FALSE(mp.match("/fo"));
  EXPECT_FALSE(mp.match("/foobar"));
  EXPECT_FALSE(mp.match("/foo/bar"));
}

TEST(MountPath, RegexCaptureParameterCapture) {
  MountPath mp = MountPath::parse_endpoint("/:[re]f([o]{2,})bar");
  auto result = mp.match("/foobar");
  ASSERT_TRUE(result.has_value());
  ASSERT_TRUE(result->contains("1"));
  EXPECT_EQ(result->at("1"), "oo");

  EXPECT_EQ(mp.match("/fooobar")->at("1"), "ooo");
  EXPECT_EQ(mp.match("/fooooooooooooooooobar")->at("1"), "ooooooooooooooooo");
  EXPECT_FALSE(mp.match("/fo"));
  EXPECT_FALSE(mp.match("/fobar"));
  EXPECT_FALSE(mp.match("/foo/bar"));
}

// -------------------------------------------------------------------------- //

class EndpointTrieTest: public ::testing::Test {
protected:
  EndpointTrieTest() {
    endpoints.reserve(10);
  }

  void insert_endpoint(EndpointTrie& trie, std::string_view endpoint) {
    endpoints.emplace_back(endpoint);
    trie.insert(MountPath::parse_endpoint(endpoint), endpoints.back());
  }

  std::vector<HttpHandlerFactory<HttpHandler>> endpoints;
};

TEST_F(EndpointTrieTest, BasicEndpoint) {
  EndpointTrie trie;
  insert_endpoint(trie, "/foo");
  auto result = trie.match("/foo");
  ASSERT_TRUE(result.has_value());
  EXPECT_TRUE(result->parameters.empty());
  EXPECT_EQ(result->endpoint.route(), "/foo");

  EXPECT_FALSE(trie.match("/bar"));
  EXPECT_FALSE(trie.match("/foobar"));
  EXPECT_FALSE(trie.match("/foo/bar"));
}

TEST_F(EndpointTrieTest, TwoPartEndpoint) {
  EndpointTrie trie;
  insert_endpoint(trie, "/foo/bar");
  auto result = trie.match("/foo/bar");
  ASSERT_TRUE(result.has_value());
  EXPECT_TRUE(result->parameters.empty());

  EXPECT_FALSE(trie.match("/bar/foo"));
}

TEST_F(EndpointTrieTest, SimpleParameterCapture) {
  EndpointTrie trie;
  insert_endpoint(trie, "/:foo");
  auto result = trie.match("/fizz");
  ASSERT_TRUE(result.has_value());
  ASSERT_TRUE(result->parameters.contains("foo"));
  EXPECT_EQ(result->parameters.at("foo"), "fizz");
  EXPECT_EQ(result->endpoint.route(), "/:foo");

  EXPECT_FALSE(trie.match("/flim/flam"));
}

TEST_F(EndpointTrieTest, DoubleParameterCapture) {
  EndpointTrie trie;
  insert_endpoint(trie, "/:foo/:bar");
  auto result = trie.match("/flim/flam");
  ASSERT_TRUE(result.has_value());
  ASSERT_TRUE(result->parameters.contains("foo"));
  ASSERT_TRUE(result->parameters.contains("bar"));
  EXPECT_EQ(result->parameters.at("foo"), "flim");
  EXPECT_EQ(result->parameters.at("bar"), "flam");
  EXPECT_EQ(result->endpoint.route(), "/:foo/:bar");

  EXPECT_FALSE(trie.match("/onepart"));
  EXPECT_FALSE(trie.match("/three/parts/here"));
}

TEST_F(EndpointTrieTest, MixedCaptureNoCapture) {
  EndpointTrie trie;
  insert_endpoint(trie, "/foo/:foo/bar/:bar/baz");
  auto result = trie.match("/foo/capture/bar/this/baz");
  ASSERT_TRUE(result.has_value());
  ASSERT_TRUE(result->parameters.contains("foo"));
  ASSERT_TRUE(result->parameters.contains("bar"));
  EXPECT_EQ(result->parameters.at("foo"), "capture");
  EXPECT_EQ(result->parameters.at("bar"), "this");
  EXPECT_EQ(result->parameters.size(), 2);
  EXPECT_EQ(result->endpoint.route(), "/foo/:foo/bar/:bar/baz");
}

TEST_F(EndpointTrieTest, IntValidatedParameterCapture) {
  EndpointTrie trie;
  insert_endpoint(trie, "/:[int]foo");
  auto result = trie.match("/1234");
  ASSERT_TRUE(result.has_value());
  ASSERT_TRUE(result->parameters.contains("foo"));
  EXPECT_EQ(result->parameters.at("foo"), "1234");
  EXPECT_EQ(result->endpoint.route(), "/:[int]foo");

  auto result2 = trie.match("/-4321");
  ASSERT_TRUE(result2.has_value());
  ASSERT_TRUE(result2->parameters.contains("foo"));
  EXPECT_EQ(result2->parameters.at("foo"), "-4321");

  EXPECT_FALSE(trie.match("/foobar"));
  EXPECT_FALSE(trie.match("/123foo"));
  EXPECT_FALSE(trie.match("/foo123"));
}

TEST_F(EndpointTrieTest, TwoMountedEndpoints) {
  EndpointTrie trie;
  insert_endpoint(trie, "/foo");
  insert_endpoint(trie, "/bar");
  auto foo_result = trie.match("/foo");
  ASSERT_TRUE(foo_result);
  EXPECT_EQ(foo_result->endpoint.route(), "/foo");

  auto bar_result = trie.match("/bar");
  ASSERT_TRUE(bar_result);
  EXPECT_EQ(bar_result->endpoint.route(), "/bar");
}

TEST_F(EndpointTrieTest, WildcardJumpBack) {
  EndpointTrie trie;
  insert_endpoint(trie, "/foo/:param2/baz");
  insert_endpoint(trie, "/:param1/:param3/other");
  auto result = trie.match("/foo/something/other");
  ASSERT_TRUE(result.has_value());
  ASSERT_TRUE(result->parameters.contains("param1"));
  ASSERT_TRUE(result->parameters.contains("param3"));

  // TODO(alaina): Remove parameters added by false paths down trie.
  // EXPECT_FALSE(result->parameters.contains(":param2"));

  EXPECT_EQ(result->parameters.at("param1"), "foo");
  EXPECT_EQ(result->parameters.at("param3"), "something");
  EXPECT_EQ(result->endpoint.route(), "/:param1/:param3/other");
}

}
}
