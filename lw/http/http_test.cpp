#include "lw/net/http.h"

#include <sstream>

#include "gtest/gtest.h"

namespace lw::net {
namespace {

TEST(HttpResponseFormat, ContentLengthGenerated) {
  HttpResponse res;
  res.status(200);
  res.body("foobar");

  std::stringstream out;
  out << res;
  EXPECT_EQ(
    out.str(),
    "HTTP/1.1 200 OK\r\n"
    "Content-Length: 6\r\n"
    "\r\n"
    "foobar"
  );
}

TEST(HttpResponseFormat, EmptyResponse) {
  HttpResponse res;
  res.status(200);

  std::stringstream out;
  out << res;
  EXPECT_EQ(
    out.str(),
    "HTTP/1.1 200 OK\r\n"
    "Content-Length: 0\r\n"
    "\r\n"
  );
}

}
}
