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
#include <map>
#include <regex>
#include <string>
#include <string_view>
#include <ostream>

#include "lw/base/strings.h"
#include "lw/co/future.h"

#define LW_REGISTER_HTTP_HANDLER(HandlerClass, method, route) \
  ::lw::net::HttpRoute<HandlerClass> _http_handler_ ## HandlerClass{method, route};

namespace lw::net {
namespace http {

typedef std::map<
  std::string_view,
  std::string_view,
  CaseInsensitiveLess
> KeyValueViewPairs;

typedef std::map<std::string, std::string, CaseInsensitiveLess> KeyValuePairs;

}

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

}
