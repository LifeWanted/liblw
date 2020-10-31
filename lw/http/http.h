#pragma once
/**
 * @file
 * To create HTTP endpoints for your server subclass `HttpHandler`, implementing
 * the `HttpHandler::run` method. Then pass your subclass to
 * `LW_REGISTER_HTTP_HANDLER` along with the HTTP verb(s) and a url matcher.
 *
 * ```cpp
 *  namespace {
 *  class MyHandler: public ::lw::net::HttpHandler {
 *  public:
 *    co::Future<Response> get() override {
 *      co_return request().route_param("endpoint");
 *    }
 *  };
 *  LW_REGISTER_HTTP_HANDLER(MyHandler, "/my/:endpoint");
 *  }
 * ```
 */

#include <istream>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <ostream>

#include "lw/base/strings.h"
#include "lw/co/task.h"
#include "lw/net/router.h"

#define LW_REGISTER_HTTP_HANDLER(HandlerClass, method, route) \
  ::lw::net::HttpRoute<HandlerClass> http_route_ ## HandlerClass{method, route};

namespace lw::net {
namespace http {

typedef std::map<
  std::string_view,
  std::string_view,
  CaseInsensitiveLess
> KeyValueViewPairs;

typedef std::map<std::string, std::string, CaseInsensitiveLess> KeyValuePairs;

}
namespace internal {

class MountPath;
class EndpointTrie;

}

class HttpHandler;
class HttpRouter;

class HttpRouteBase: public RouteBase {
public:
  HttpRouteBase(std::string_view method, std::string_view endpoint):
    _method{method},
    _endpoint{endpoint}
  {
    ::lw::net::register_route<HttpRouter>(this);
  }

  virtual ~HttpRouteBase() = default;

  std::string_view method() const { return _method; }
  std::string_view endpoint() const { return _endpoint; }

  virtual std::unique_ptr<HttpHandler> make_handler() = 0;

private:
  std::string_view _method;
  std::string_view _endpoint;
};

template <typename HttpRouteHandler>
class HttpRoute: public HttpRouteBase {
public:
  HttpRoute(std::string_view method, std::string_view endpoint):
    HttpRouteBase{method, endpoint}
  {}
  ~HttpRoute() = default;

  std::unique_ptr<HttpHandler> make_handler() override {
    return std::make_unique<HttpRouteHandler>();
  }
};


// /**
//  * Base class for HTTP request handlers. A new instance is created for each
//  * request, so instance members should be used for per-request state.
//  */
// class HttpHandler: public HandlerBase {
// public:
//   virtual std::future<HttpResponse::Body> run() = 0;
//
//   const HttpRequest& request() const { return _request; }
//   const HttpResponse& response() const { return _response; }
//   HttpResponse& response() { return _response; }
//
//   template <typename... Args>
//   std::future<HttpResponse::Body> future_response_body(Args&&... args) const {
//     return co::make_future(HttpResponse::Body{std::forward<Args>(args)...});
//   }
//
// protected:
//   std::future<void> raw_run() override;
//
// private:
//   HttpRequest _request;
//   HttpResponse _response;
// };
//
// class HttpRoute: public Route<HttpRouter, HttpResponse> {
//
// };

class HttpRouter: public Router {
public:
  void attach_routes() override;
  co::Task<void> run(std::unique_ptr<Socket> conn) override;
  std::size_t connection_count() const override;

private:
  void mount_route(const http::MountPath& path, HttpRouteBase* route);

};

}
