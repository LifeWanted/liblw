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
 *    std::future<lw::net::HttpResponse::Body> run() override {
 *      return future_response_body(request().route_param("endpoint"));
 *    }
 *  };
 *  LW_REGISTER_HTTP_HANDLER(MyHandler, "GET", "/my/:endpoint");
 *  }
 * ```
 *
 */

#include <future>
#include <istream>
#include <list>
#include <map>
#include <string>
#include <string_view>
#include <ostream>

#include "lw/base/strings.h"
#include "lw/co/future.h"

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

class MountPath;

class PathPart {
public:
  PathPart(std::string_view name, bool is_exact):
    _name{name}, _is_exact{is_exact}
  {}

  std::string_view get_match_at_start(std::string_view path);

private:
  std::string_view _name;
  bool _is_exact;
  std::list<PathPart> _children;
};
}

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

/**
 *
 */
class HttpRequest {
public:
  explicit HttpRequest(std::istream* connection);

  std::string_view http_version() const { return _http_version; }
  std::string_view method() const { return _method; }
  std::string_view raw_header() const { return _raw_header; }
  std::string_view path() const { return _path; }
  std::string_view raw_path() const { return _raw_path; }

  bool has_header(std::string_view header_name) const {
    return _headers.contains(header_name);
  }
  std::string_view header(std::string_view header_name) const {
    return _headers.at(header_name);
  }

  bool has_query_param(std::string_view param_name) const {
    return _query_params.contains(param_name);
  }
  std::string_view query_param(std::string_view param_name) const {
    return _query_params.at(param_name);
  }

  std::size_t content_length() const { return _content_length; }

  [[nodiscard]] std::future<void> read_header();

  /**
   * Reads and discards the remaining data for the request.
   */
  void consume_request();

private:
  std::size_t _parse_method_line(std::string_view header_view);
  void _parse_headers(std::string_view header_view);
  void _parse_content_length();

  std::istream* _connection;

  std::string _raw_header;
  std::string_view _http_version;
  std::string_view _method;
  std::string_view _path;
  std::string_view _raw_path;

  http::KeyValueViewPairs _headers;
  http::KeyValueViewPairs _query_params;

  std::int64_t _content_length = -1;
};

class HttpResponse {
public:
  // TODO: Turn this into a more advanced class capable of handling streams and
  // serializing based on MIME type.
  typedef std::string Body;

  int status() const { return _status_code; }

  void status(int code) {
    _status_code = code;
    _status_message.clear();
  }

  void status(int code, std::string_view message) {
    _status_code = code;
    _status_message = message;
  }

  void status(int code, std::string&& message) {
    _status_code = code;
    _status_message = std::move(message);
  }

  std::string_view status_message() const;

  bool has_header(std::string_view header_name) const {
    return _headers.contains(header_name);
  }

  const std::string& header(std::string_view header_name) const;

  void header(std::string_view header_name, std::string_view value);

  const http::KeyValuePairs& headers() const { return _headers; }

  void body(const Body& b) { _body = b; }
  void body(Body&& b) { _body = std::move(b); }
  const Body& body() const { return _body; }

private:
  int _status_code = 0;
  std::string _status_message;
  http::KeyValuePairs _headers;
  Body _body;
};

std::ostream& operator<<(std::ostream& stream, const HttpResponse& response);

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
  [[nodiscard]] std::future<void> run(Socket* conn) override;

private:
  void mount_route(const http::MountPath& path, HttpRoute* route);

};

}
