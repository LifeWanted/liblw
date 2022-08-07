#pragma once

#include <string_view>

#include "lw/co/future.h"
#include "lw/http/http_request.h"

namespace lw {

class RestRequest {
public:
  explicit RestRequest(const HttpRequest& http_req):
    _http{&http_req}
  {}

  std::string_view http_version() const { return _http->http_version(); }
  std::string_view method() const { return _http->method(); }
  std::string_view raw_header() const { return _http->raw_header(); }
  std::string_view path() const { return _http->path(); }
  std::string_view raw_path() const { return _http->raw_path(); }

  bool has_header(std::string_view header_name) const {
    return _http->has_header(header_name);
  }

  std::string_view header(std::string_view header_name) const {
    return _http->header(header_name);
  }

  bool has_query_param(std::string_view param_name) const {
    return _http->has_query_param(param_name);
  }
  std::string_view query_param(std::string_view param_name) const {
    return _http->query_param(param_name);
  }

  bool has_route_param(std::string_view param_name) const {
    return _http->has_route_param(param_name);
  }
  std::string_view route_param(std::string_view param_name) const {
    return _http->route_param(param_name);
  }

  std::size_t content_length() const { return _http->content_length(); }

  co::Future<Buffer> body() const;

private:
  const HttpRequest* _http = nullptr;
};

}
