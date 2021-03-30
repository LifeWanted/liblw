#pragma once

#include <memory>

#include "lw/co/await.h"
#include "lw/co/future.h"
#include "lw/co/task.h"
#include "lw/net/router.h"
#include "lw/net/tls.h"
#include "lw/net/tls_options.h"

namespace lw::net {
namespace internal {

co::Future<std::unique_ptr<net::TLSStream>> tls_wrap_connection(
  net::TLSStreamFactory& tls_factory,
  std::unique_ptr<io::CoStream> conn
);

}

struct TLSRouterOptions {
  // TODO(alaina): Add utility for self-signed certs in memory and support for
  // passing a private key here.
  std::variant<std::filesystem::path> private_key;
  std::variant<std::filesystem::path> certificate;
};

template <typename BaseRouter, typename Options = ::lw::net::TLSRouterOptions>
class TLSRouter: public Router {
public:
  typedef BaseRouter base_router_t;
  typedef Options options_t;

  TLSRouter(const options_t& options):
    _router{options},
    _tls_factory{{
      .private_key = options.tls.private_key,
      .certificate = options.tls.certificate,
      .connection_mode = TLSOptions::ACCEPT
    }}
  {}

  BaseRouter& base_router() { return _router; }
  const BaseRouter& base_router() const { return _router; }

  void attach_routes() override { _router.attach_routes(); }
  std::size_t connection_count() const override {
    return _router.connection_count();
  }

  co::Task run(std::unique_ptr<io::CoStream> conn) override {
    auto tls_conn = co_await internal::tls_wrap_connection(
      _tls_factory,
      std::move(conn)
    );
    if (tls_conn) co_await _router.run(std::move(tls_conn));
  }

private:
  BaseRouter _router;
  TLSStreamFactory _tls_factory;
};

template <typename BaseRouter>
class TLSRouter<BaseRouter, TLSRouterOptions>: public Router {
public:
  typedef BaseRouter base_router_t;
  typedef TLSRouterOptions options_t;

  TLSRouter(const options_t& options):
    _router{},
    _tls_factory{{
      .private_key = options.private_key,
      .certificate = options.certificate,
      .connection_mode = TLSOptions::ACCEPT
    }}
  {}

  BaseRouter& base_router() { return _router; }
  const BaseRouter& base_router() const { return _router; }

  void attach_routes() override { _router.attach_routes(); }
  std::size_t connection_count() const override {
    return _router.connection_count();
  }

  co::Task run(std::unique_ptr<io::CoStream> conn) override {
    auto tls_conn = co_await internal::tls_wrap_connection(
      _tls_factory,
      std::move(conn)
    );
    if (tls_conn) {
      co_await co::task_completion(_router.run(std::move(tls_conn)));
    }
  }

private:
  BaseRouter _router;
  TLSStreamFactory _tls_factory;
};

}
