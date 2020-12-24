#pragma once
/**
 * @file
 * To create HTTP endpoints for your server subclass `HttpHandler`, implementing
 * the `HttpHandler::run` method. Then pass your subclass to
 * `LW_REGISTER_HTTP_HANDLER` along with the HTTP verb(s) and a url matcher.
 *
 * ```cpp
 *  namespace {
 *  class MyHandler: public ::lw::HttpHandler {
 *  public:
 *    co::Future<void> get() override {
 *      response().body(request().route_param("endpoint"));
 *      co_return;
 *    }
 *  };
 *  LW_REGISTER_HTTP_HANDLER(MyHandler, "/my/:endpoint");
 *  }
 * ```
 */

#include <memory>

#include "lw/co/task.h"
#include "lw/http/http_handler.h"
#include "lw/http/internal/http_mount_path.h"
#include "lw/io/co/co.h"
#include "lw/net/router.h"

#define LW_REGISTER_HTTP_HANDLER(HandlerClass, route) \
  ::lw::HttpRoute http_route_ ## HandlerClass{        \
    ::lw::HttpHandlerFactory<HandlerClass>{route}     \
  };

namespace lw {

class HttpRoute: public net::RouteBase {
public:
  template <typename HttpHandlerFactoryType>
  explicit HttpRoute(HttpHandlerFactoryType&& factory);

  const BaseHttpHandlerFactory& factory() const { return *_factory; }

private:
  std::unique_ptr<BaseHttpHandlerFactory> _factory;
};

class HttpRouter: public net::Router {
public:
  void attach_routes() override;
  co::Task<void> run(std::unique_ptr<io::CoStream> conn) override;
  std::size_t connection_count() const override { return _connection_counter; }

  co::Future<void> run_once(io::CoStream& conn);

private:
  http::internal::EndpointTrie _trie;
  std::size_t _connection_counter = 0;
};

template <typename HttpHandlerFactoryType>
HttpRoute::HttpRoute(HttpHandlerFactoryType&& factory) {
  register_route<HttpRouter>(this);
  _factory = std::make_unique<HttpHandlerFactoryType>(
    std::forward<HttpHandlerFactoryType>(factory)
  );
}

}
