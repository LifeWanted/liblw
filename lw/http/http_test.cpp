#include "lw/http/http.h"

#include <sstream>

#include "gtest/gtest.h"
#include "lw/co/scheduler.h"
#include "lw/http/http_handler.h"
#include "lw/io/co/testing/string_stream.h"

namespace lw {
namespace {

using ::lw::io::testing::CoStringStream;

class TestHttpHandler: public HttpHandler {
public:
  co::Future<void> get() override {
    response().body(request().route_param("endpoint"));
    co_return;
  }
};
LW_REGISTER_HTTP_HANDLER(TestHttpHandler, "/test/:endpoint");

TEST(HttpRouter, ExecutesRegisteredHandlers) {
  HttpRouter router;
  router.attach_routes();

  std::string response;
  auto conn = std::make_unique<CoStringStream>(
    "GET /test/foobar HTTP/1.1\r\n"
    "Host: localhost\r\n\r\n",
    response
  );
  co::Scheduler::this_thread().schedule(router.run(std::move(conn)));
  co::Scheduler::this_thread().run();

  EXPECT_EQ(
    response,
    "HTTP/1.1 200 OK\r\n"
    "Content-Length: 6\r\n"
    "\r\n"
    "foobar"
  );
}

TEST(HttpRouter, RespondsNotFOund) {
  HttpRouter router;
  router.attach_routes();

  std::string response;
  auto conn = std::make_unique<CoStringStream>(
    "GET /gibberish HTTP/1.1\r\n"
    "Host: localhost\r\n\r\n",
    response
  );
  co::Scheduler::this_thread().schedule(router.run(std::move(conn)));
  co::Scheduler::this_thread().run();

  EXPECT_EQ(
    response,
    "HTTP/1.1 404 Not Found\r\n"
    "Content-Type: text/plain\r\n"
    "Content-Length: 10\r\n"
    "\r\n"
    "Not Found."
  );
}

}
}
