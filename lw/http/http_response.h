#pragma once

#include <ostream>
#include <string>
#include <string_view>

#include "lw/http/headers.h"
#include "lw/http/http_status.h"
#include "lw/memory/buffer.h"

namespace lw {

class HttpResponse {
public:
  // TODO: #16 Turn this into a more advanced class capable of handling streams
  // and serializing based on MIME type.
  using Body = std::string;

  int status() const {
    return _status_code ? _status_code : to_int(HttpStatus::OK);
  }

  void status(HttpStatus code) { status(to_int(code)); }
  void status(HttpStatus code, std::string_view message) {
    status(to_int(code), message);
  }
  void status(HttpStatus code, std::string&& message) {
    status(to_int(code), std::move(message));
  }

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

  std::string_view header(std::string_view header_name) const;

  void header(std::string_view header_name, std::string_view value);

  const http::Headers& headers() const { return _headers; }

  void body(std::string_view b) { _body = Body{b}; }
  const Body& body() const { return _body; }

  Buffer serialize() const;

private:
  int _status_code = 0;
  std::string _status_message;
  http::Headers _headers;
  Body _body;
};

std::ostream& operator<<(std::ostream& stream, const HttpResponse& response);

} // namespace lw
