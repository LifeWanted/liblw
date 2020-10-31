#pragma once

#include <string>
#include <string_view>
#include <ostream>

#include "lw/http/headers.h"
#include "lw/memory/buffer.h"

namespace lw {

class HttpResponse {
public:
  enum Status {
    CONTINUE                        = 100,
    SWITCHING_PROTOCOLS             = 101,
    PROCESSING                      = 102,
    EARLY_HINTS                     = 103,

    OK                              = 200,
    CREATED                         = 201,
    ACCEPTED                        = 202,
    NONAUTHORITATIVE_INFORMATION    = 203,
    NO_CONTENT                      = 204,
    RESET_CONTENT                   = 205,
    PARTIAL_CONTENT                 = 206,
    MULTISTATUS                     = 207,
    ALREADY_REPORTED                = 208,
    IM_USED                         = 226,

    MULTIPLE_CHOICES                = 300,
    MOVED_PERMANENTLY               = 301,
    FOUND                           = 302,
    SEE_OTHER                       = 303,
    NOT_MODIFIED                    = 304,
    USE_PROXY                       = 305,
    SWITCH_PROXY                    = 306,
    TEMPORARY_REDIRECT              = 307,
    PERMANENT_REDIRECT              = 308,

    BAD_REQUEST                     = 400,
    UNAUTHORIZED                    = 401,
    PAYMENT_REQUIRED                = 402,
    FORBIDDEN                       = 403,
    NOT_FOUND                       = 404,
    METHOD_NOT_ALLOWED              = 405,
    NOT_ACCEPTABLE                  = 406,
    PROXY_AUTHENTICATION_REQUIRED   = 407,
    REQUEST_TIMEOUT                 = 408,
    CONFLICT                        = 409,
    GONE                            = 410,
    LENGTH_REQUIRED                 = 411,
    PRECONDITION_FAILED             = 412,
    PAYLOAD_TOO_LARGE               = 413,
    URI_TOO_LONG                    = 414,
    UNSUPPORTED_MEDIA_TYPE          = 415,
    RANGE_NOT_SATISFIABLE           = 416,
    EXPECTATION_FAILED              = 417,
    IM_A_TEAPOT                     = 418,
    MISDIRECTED_REQUEST             = 421,
    UNPROCESSABLE_ENTITY            = 422,
    LOCKED                          = 423,
    FAILED_DEPENDENCY               = 424,
    TOO_EARLY                       = 425,
    UPGRADE_REQUIRED                = 426,
    PRECONDITION_REQUIRED           = 428,
    TOO_MANY_REQUESTS               = 429,
    REQUEST_HEADER_FIELDS_TOO_LARGE = 431,
    UNAVAILABLE_FOR_LEGAL_REASONS   = 451,

    INTERNAL_SERVER_ERROR           = 500,
    NOT_IMPLEMENTED                 = 501,
    BAD_GATEWAY                     = 502,
    SERVICE_UNAVAILABLE             = 503,
    GATEWAY_TIMEOUT                 = 504,
    HTTP_VERSION_NOT_SUPPORTED      = 505,
    VARIANT_ALSO_NEGOTIATES         = 506,
    INSUFFICIENT_STORAGE            = 507,
    LOOP_DETECTED                   = 508,
    NOT_EXTENDED                    = 510,
    NETWORK_AUTHENTICATION_REQUIRED = 511,
  };

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

  std::string_view header(std::string_view header_name) const;

  void header(std::string_view header_name, std::string_view value);

  const http::Headers& headers() const { return _headers; }

  void body(const Body& b) { _body = b; }
  void body(Body&& b) { _body = std::move(b); }
  const Body& body() const { return _body; }

  Buffer serialize() const;

private:
  int _status_code = 0;
  std::string _status_message;
  http::Headers _headers;
  Body _body;
};

std::ostream& operator<<(std::ostream& stream, const HttpResponse& response);

}
