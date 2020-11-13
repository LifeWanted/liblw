#pragma once

#include <filesystem>
#include <memory>
#include <variant>

#include "lw/co/task.h"
#include "lw/http/http.h"
#include "lw/io/co/co.h"
#include "lw/net/router.h"

namespace lw {
namespace internal {

class TLSState;

}

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
  std::size_t connection_count() const override {
    return _http_router.connection_count();
  }

  co::Task<void> run(std::unique_ptr<io::CoStream> conn) override;

private:
  HttpRouter _http_router;
  std::unique_ptr<internal::TLSState> _tls;
};

}
