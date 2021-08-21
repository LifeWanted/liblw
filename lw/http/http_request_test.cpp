#include "lw/http/http_request.h"

#include <sstream>

#include "gtest/gtest.h"
#include "lw/co/scheduler.h"
#include "lw/co/task.h"
#include "lw/io/co/testing/string_reader.h"

namespace lw {
namespace {

using ::lw::io::testing::StringReader;

template <typename Func>
void run(Func&& coroutine) {
  co::Scheduler::this_thread().schedule(std::forward<Func>(coroutine));
  co::Scheduler::this_thread().run();
}

TEST(HttpRequestReadHeader, ParsesHeader) {
  run([]() -> co::Task {
    StringReader input{
      "GET /foo/bar HTTP/1.1\r\n"
      "Host: test.com\r\n"
      "Content-Length: 42\r\n"
      "\r\n"
    };
    HttpRequest req{input};
    co_await req.read_header();

    EXPECT_EQ(req.method(), "GET");
    EXPECT_EQ(req.path(), "/foo/bar");
    EXPECT_EQ(req.http_version(), "HTTP/1.1");
    EXPECT_TRUE(req.has_header("Host"));
    EXPECT_EQ(req.header("Host"), "test.com");
    EXPECT_EQ(req.header("Content-Length"), "42");
  });
}

TEST(HttpRequestReadHeader, ParsesContentLength) {
  run([]() -> co::Task {
    StringReader input{
      "GET /foo/bar HTTP/1.1\r\n"
      "Host: test.com\r\n"
      "Content-Length: 42\r\n"
      "\r\n"
    };
    HttpRequest req{input};
    co_await req.read_header();

    EXPECT_EQ(req.content_length(), 42);
  });
}

TEST(HttpRequestReadHeader, ParsesQueryString) {
  run([]() -> co::Task {
    StringReader input{
      "GET /foo/bar?fizz=bang&&foo=&bar=42&life&another HTTP/1.1\r\n"
      "Host: test.com\r\n"
      "\r\n"
    };
    HttpRequest req{input};
    co_await req.read_header();

    EXPECT_TRUE(req.has_query_param("fizz"));
    EXPECT_TRUE(req.has_query_param("foo"));
    EXPECT_TRUE(req.has_query_param("bar"));
    EXPECT_TRUE(req.has_query_param("life"));
    EXPECT_TRUE(req.has_query_param("another"));
    EXPECT_EQ(req.query_param("fizz"), "bang");
    EXPECT_EQ(req.query_param("foo"), "");
    EXPECT_EQ(req.query_param("bar"), "42");
    EXPECT_EQ(req.query_param("life"), "");
    EXPECT_EQ(req.query_param("another"), "");
  });
}

TEST(HttpRequestHeaders, HasHeaderIsTrueForProvidedHeaders) {
  run([]() -> co::Task {
    StringReader input{
      "GET /foo/bar HTTP/1.1\r\n"
      "Host: test.com\r\n"
      "\r\n"
    };
    HttpRequest req{input};
    co_await req.read_header();

    EXPECT_TRUE(req.has_header("Host"));
  });
}

TEST(HttpRequestHeaders, HasHeaderIsFalseForMissingHeaders) {
  run([]() -> co::Task {
    StringReader input{
      "GET /foo/bar HTTP/1.1\r\n"
      "Host: test.com\r\n"
      "\r\n"
    };
    HttpRequest req{input};
    co_await req.read_header();

    EXPECT_FALSE(req.has_header("this-header-is-not-provided"));
  });
}

TEST(HttpRequestHeaders, IncludeEmptyHeaders) {
  run([]() -> co::Task {
    StringReader input{
      "GET /foo/bar HTTP/1.1\r\n"
      "Host: test.com\r\n"
      "A-Header:\r\n"
      "Spacey-Header: \t\r\n"
      "\r\n"
    };
    HttpRequest req{input};
    co_await req.read_header();

    EXPECT_TRUE(req.has_header("A-Header"));
    EXPECT_TRUE(req.has_header("Spacey-Header"));

    EXPECT_EQ(req.header("A-Header"), "");
    EXPECT_EQ(req.header("Spacey-Header"), "");
  });
}

TEST(HttpRequestHeaders, AreCaseInsensitive) {
  run([]() -> co::Task {
    StringReader input{
      "GET /foo/bar HTTP/1.1\r\n"
      "Host: test.com\r\n"
      "\r\n"
    };
    HttpRequest req{input};
    co_await req.read_header();

    EXPECT_TRUE(req.has_header("Host"));
    EXPECT_TRUE(req.has_header("host"));
    EXPECT_TRUE(req.has_header("hoST"));
    EXPECT_TRUE(req.has_header("HOST"));

    EXPECT_EQ(req.header("Host"), "test.com");
    EXPECT_EQ(req.header("host"), "test.com");
    EXPECT_EQ(req.header("hoST"), "test.com");
    EXPECT_EQ(req.header("HOST"), "test.com");
  });
}

TEST(HttpRequestBody, IsReadableFromContentLength) {
  run([]() -> co::Task {
    StringReader input{
      "POST /foo/bar HTTP/1.1\r\n"
      "Host: test.com\r\n"
      "Content-Length: 6\r\n"
      "\r\n"
      "foobar"
    };
    HttpRequest req{input};
    co_await req.read_header();

    Buffer body = co_await req.body();
    EXPECT_EQ(static_cast<std::string_view>(body), "foobar");
  });
}

}
}
