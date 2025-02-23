#pragma once

#include <ostream>
#include <string_view>

#include "lw/http/headers.h"
#include "lw/http/http_body.h"
#include "lw/http/http_method.h"
#include "lw/memory/buffer.h"

namespace lw {

/**
 * Outgoing HTTP request to a remove service.
 *
 * Composed in this class and then serialized and sent over a connection.
 */
class HttpClientRequest {
public:
  HttpClientRequest(
      HttpMethod method, std::string_view host, std::string_view path)
      : HttpClientRequest(to_string(method), host, path) {}

  HttpClientRequest(
      std::string_view method, std::string_view host, std::string_view path);

  std::string_view method() const { return _method; }
  std::string_view path() const { return _path; }

  bool has_header(std::string_view header_name) const {
    return _headers.contains(header_name);
  }

  std::string_view header(std::string_view header_name) const;

  void header(std::string_view header_name, std::string_view value);

  const http::Headers& headers() const { return _headers; }

  void body(HttpBody content) { _body = std::move(content); }
  void body(std::string_view content) { _body = HttpBody::text(content); }
  const HttpBody& body() const { return _body; }

  Buffer serialize() const;

private:
  std::string _method;
  std::string _path;
  http::Headers _headers;
  HttpBody _body;
};

std::ostream& operator<<(std::ostream& stream, const HttpClientRequest& req);

} // namespace lw
