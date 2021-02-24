/**
 * An example HTTP echo server which responds to `POST /echo` requests with the
 * contents of the request and `GET /echo` with the query parameters.
 *
 * If `--tls-cert-path` and `--tls-key-path` are provided, then an HTTPS server
 * will be created. Otherwise an HTTP server (no encryption) is started.
 */

#include <memory>
#include <stdexcept>
#include <string>

#include "lw/base/base.h"
#include "lw/co/future.h"
#include "lw/flags/flags.h"
#include "lw/http/http.h"
#include "lw/http/https.h"
#include "lw/log/log.h"
#include "lw/net/server.h"

LW_FLAG(
  unsigned short, port, 8080,
  "Port for the server to listen on for HTTP."
);
LW_FLAG(
  std::string, tls_cert_path, "",
  "Path to the TLS certificate."
);
LW_FLAG(
  std::string, tls_key_path, "",
  "Path to the TLS private key."
);

namespace {
class EchoHandler: public lw::HttpHandler {
public:
  lw::co::Future<void> get() override {
    response().header(
      "content-length",
      std::to_string(request().raw_path().size())
    );
    response().body(request().raw_path());
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
  // Routers must outlive the server, so we'll declare them outside the try.
  std::unique_ptr<lw::net::Router> router;
  try {
    if (!lw::init(&argc, argv)) {
      return 0;
    }
    lw::net::Server server;

    // Enable HTTPS routing only if certificate and key are provided.
    if (
      !lw::flags::tls_cert_path.value().empty() &&
      !lw::flags::tls_key_path.value().empty()
    ) {
      lw::log(lw::INFO) << "Creating HTTPS router.";
      router = std::make_unique<lw::HttpsRouter>(
        lw::HttpsRouter::Options{
          .private_key = lw::flags::tls_key_path.value(),
          .certificate = lw::flags::tls_cert_path.value()
        }
      );
    } else {
      lw::log(lw::INFO) << "Creating HTTP router.";
      router = std::make_unique<lw::HttpRouter>();
    }

    server.attach_router(lw::flags::port, router.get());
    server.listen();
    lw::log(lw::INFO) << "Server is listening on " << lw::flags::port;
    server.run();
  } catch (const std::runtime_error& err) {
    lw::log(lw::ERROR) << err.what();
    return -1;
  }
  return 0;
}
