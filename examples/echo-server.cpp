/**
 * @file
 *  An example http echo server which responds to `POST /echo` requests with the
 *  contents of the request.
 */

#include <future>
#include <iostream>
#include <stdexcept>

#include "lw/base/base.h"
#include "lw/co/future.h"
#include "lw/flags/flags.h"
#include "lw/net/server.h"
#include "lw/http/http.h"

LW_FLAG(unsigned short, port, 8080, "Port for the server to listen on.");

namespace {
class EchoHandler: public lw::HttpHandler {
public:
  lw::co::Future<void> get() override {
    if (request().has_header("content-length")) {
      response().header("content-length", request().header("content-length"));
    }
    response().body(request().path());
    co_return;
  }

  lw::co::Future<void> post() override {
    if (request().has_header("content-length")) {
      response().header("content-length", request().header("content-length"));
    }
    response().body(co_await request().body());
    co_return;
  }
};

LW_REGISTER_HTTP_HANDLER(EchoHandler, "/echo");

}

int main(int argc, const char** argv) {
  try {
    if (!lw::init(&argc, argv)) {
      return 0;
    }

    lw::net::Server server;
    lw::HttpRouter router;
    server.attach_router(lw::flags::port, &router);

    server.listen();
    std::cout << "Server is listening on port " << lw::flags::port << std::endl;
    server.run();
  } catch (const std::runtime_error& err) {
    std::cerr << err.what() << std::endl;
    return -1;
  }
  return 0;
}
