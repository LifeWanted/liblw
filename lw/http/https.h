#pragma once

#include <filesystem>
#include <memory>
#include <variant>

#include "lw/co/future.h"
#include "lw/co/task.h"
#include "lw/http/http.h"
#include "lw/io/co/co.h"
#include "lw/net/router.h"
#include "lw/net/tls.h"

namespace lw {

class HttpsRouter: public net::Router {
public:
  struct Options {
    // TODO(alaina): Add utility for self-signed certs in memory and support for
    // passing a private key here.
    std::variant<std::filesystem::path> private_key;
    std::variant<std::filesystem::path> certificate;
  };

  HttpsRouter(const Options& options);

  void attach_routes() override { _http_router.attach_routes(); }
  std::size_t connection_count() const override { return _connection_counter; }

  co::Task<void> run(std::unique_ptr<io::CoStream> conn) override;

private:
  HttpRouter _http_router;
  net::TLSStreamFactory _tls;
  std::size_t _connection_counter = 0;
};

}
