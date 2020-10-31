#pragma once

#include <istream>
#include <string>
#include <string_view>

#include "lw/co/future.h"
#include "lw/http/headers.h"
#include "lw/io/co/co.h"

namespace lw {

/**
 *
 */
class HttpRequest {
public:
  explicit HttpRequest(io::BaseCoReader& connection):
    _connection{connection}
  {}

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

  co::Future<void> read_header();

  /**
   * Reads and discards the remaining data for the request.
   */
  void consume_request();

private:
  std::size_t _parse_method_line(std::string_view header_view);
  void _parse_headers(std::string_view header_view);
  void _parse_content_length();

  io::BaseCoReader& _connection;

  std::string _raw_header;
  std::string_view _http_version;
  std::string_view _method;
  std::string_view _path;
  std::string_view _raw_path;

  http::HeadersView _headers;
  http::HeadersView _query_params;

  std::int64_t _content_length = -1;
};

}
