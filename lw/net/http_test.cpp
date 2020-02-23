#include "lw/net/http.h"

#include <sstream>

#include "gtest/gtest.h"

namespace lw::net {
namespace {

TEST(HttpRequestReadHeader, ParsesHeader) {
  std::stringstream input{
    "GET /foo/bar HTTP/1.1\r\n"
    "Host: test.com\r\n"
    "Content-Length: 42\r\n"
    "\r\n"
  };
  HttpRequest req{&input};
  req.read_header().get();

  EXPECT_EQ(req.method(), "GET");
  EXPECT_EQ(req.path(), "/foo/bar");
  EXPECT_EQ(req.http_version(), "HTTP/1.1");
  EXPECT_TRUE(req.has_header("Host"));
  EXPECT_EQ(req.header("Host"), "test.com");
  EXPECT_EQ(req.header("Content-Length"), "42");
}

TEST(HttpRequestReadHeader, ParsesContentLength) {
  std::stringstream input{
    "GET /foo/bar HTTP/1.1\r\n"
    "Host: test.com\r\n"
    "Content-Length: 42\r\n"
    "\r\n"
  };
  HttpRequest req{&input};
  req.read_header().get();

  EXPECT_EQ(req.content_length(), 42);
}

TEST(HttpRequestReadHeader, ParsesQueryString) {
  std::stringstream input{
    "GET /foo/bar?fizz=bang&&foo=&bar=42&life&another HTTP/1.1\r\n"
    "Host: test.com\r\n"
    "\r\n"
  };
  HttpRequest req{&input};
  req.read_header().get();

  ASSERT_TRUE(req.has_query_param("fizz"));
  ASSERT_TRUE(req.has_query_param("foo"));
  ASSERT_TRUE(req.has_query_param("bar"));
  ASSERT_TRUE(req.has_query_param("life"));
  ASSERT_TRUE(req.has_query_param("another"));
  EXPECT_EQ(req.query_param("fizz"), "bang");
  EXPECT_EQ(req.query_param("foo"), "");
  EXPECT_EQ(req.query_param("bar"), "42");
  EXPECT_EQ(req.query_param("life"), "");
  EXPECT_EQ(req.query_param("another"), "");
}

TEST(HttpRequestHeaders, HasHeaderIsTrueForProvidedHeaders) {
  std::stringstream input{
    "GET /foo/bar HTTP/1.1\r\n"
    "Host: test.com\r\n"
    "\r\n"
  };
  HttpRequest req{&input};
  req.read_header().get();

  EXPECT_TRUE(req.has_header("Host"));
}

TEST(HttpRequestHeaders, HasHeaderIsFalseForMissingHeaders) {
  std::stringstream input{
    "GET /foo/bar HTTP/1.1\r\n"
    "Host: test.com\r\n"
    "\r\n"
  };
  HttpRequest req{&input};
  req.read_header().get();

  EXPECT_FALSE(req.has_header("this-header-is-not-provided"));
}

TEST(HttpRequestHeaders, IncludeEmptyHeaders) {
  std::stringstream input{
    "GET /foo/bar HTTP/1.1\r\n"
    "Host: test.com\r\n"
    "A-Header:\r\n"
    "Spacey-Header: \t\r\n"
    "\r\n"
  };
  HttpRequest req{&input};
  req.read_header().get();

  ASSERT_TRUE(req.has_header("A-Header"));
  ASSERT_TRUE(req.has_header("Spacey-Header"));

  EXPECT_EQ(req.header("A-Header"), "");
  EXPECT_EQ(req.header("Spacey-Header"), "");
}

TEST(HttpRequestHeaders, AreCaseInsensitive) {
  std::stringstream input{
    "GET /foo/bar HTTP/1.1\r\n"
    "Host: test.com\r\n"
    "\r\n"
  };
  HttpRequest req{&input};
  req.read_header().get();

  EXPECT_TRUE(req.has_header("Host"));
  EXPECT_TRUE(req.has_header("host"));
  EXPECT_TRUE(req.has_header("hoST"));
  EXPECT_TRUE(req.has_header("HOST"));

  EXPECT_EQ(req.header("Host"), "test.com");
  EXPECT_EQ(req.header("host"), "test.com");
  EXPECT_EQ(req.header("hoST"), "test.com");
  EXPECT_EQ(req.header("HOST"), "test.com");
}

}
}
