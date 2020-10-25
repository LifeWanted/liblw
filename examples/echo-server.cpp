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
#include "lw/net/http.h"

LW_FLAG(unsigned short, port, 8080, "Port for the server to listen on.");

namespace {
class EchoHandler: public lw::net::HttpHandler {
public:
  lw::co::Future<lw::net::HttpResponse::Body> run() override {
    if (request().has_header("content-length")) {
      response().header("content-length", request().header("content-length"));
    }
    co_return request().body();
  }
};
LW_REGISTER_HTTP_HANDLER(EchoHandler, "POST", "/echo");
}

int main(int argc, char** argv) {
  try {
    if (!lw::init(&argc, argv)) {
      return 0;
    }

    lw::net::Server server;
    lw::net::HttpRouter router;
    server.attach_router(lw::flags::port, router);

    server.listen();
    std::cout << "Server is listening on port " << lw::flags::port << std::endl;
    server.run();
  } catch (const std::runtime_error& err) {
    std::cerr << err.what() << std::endl;
    return -1;
  }
  return 0;
}
