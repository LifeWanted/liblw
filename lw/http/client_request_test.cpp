#include "lw/http/client_request.h"

#include <sstream>
#include <string>

#include "lw/http/http_method.h"
#include "gtest/gtest.h"

namespace lw {
namespace {

std::string to_string(const HttpClientRequest& req) {
  std::stringstream out;
  out << req;
  return out.str();
}

TEST(HttpClientRequest, GetRequest) {
  HttpClientRequest req(HttpMethod::GET, "example.com", "/a/path");
  EXPECT_EQ(
      to_string(req),
      "GET /a/path HTTP/1.1\r\n"
      "Host: example.com\r\n"
      "\r\n");
}

TEST(HttpClientRequest, SetHeaders) {
  HttpClientRequest req(HttpMethod::GET, "example.com", "/a/path");
  req.header("foo", "bar");

  EXPECT_EQ(
      to_string(req),
      "GET /a/path HTTP/1.1\r\n"
      "foo: bar\r\n"
      "Host: example.com\r\n"
      "\r\n");
}

TEST(HttpClientRequest, SetBody) {
  HttpClientRequest req(HttpMethod::GET, "example.com", "/a/path");
  req.body("foobar");

  EXPECT_EQ(
      to_string(req),
      "GET /a/path HTTP/1.1\r\n"
      "Host: example.com\r\n"
      "Content-Length: 6\r\n"
      "\r\n"
      "foobar");
}

} // namespace
} // namespace lw
