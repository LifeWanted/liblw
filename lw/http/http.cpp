#include "lw/net/http.h"

#include <cctype>
#include <charconv>
#include <experimental/source_location>
#include <future>
#include <ios>
#include <memory>
#include <optional>
#include <regex>
#include <string_view>
#include <vector>

#include "lw/base/strings.h"
#include "lw/co/task.h"
#include "lw/err/canonical.h"
#include "lw/err/macros.h"
#include "lw/net/internal/http_mount_path.h"

namespace lw::net {
namespace {

std::string_view default_status_message(int code) {
  typedef std::unordered_map<int, const char*> StatusMessageMap;
  static const StatusMessageMap* _messages = new StatusMessageMap{
    {100, "Continue"},
    {101, "Switching Protocols"},
    {102, "Processing"},
    {103, "Early Hints"},
    {200, "OK"},
    {201, "Created"},
    {202, "Accepted"},
    {203, "Non-Authoritative Information"},
    {204, "No Content"},
    {205, "Reset Content"},
    {206, "Partial Content"},
    {207, "Multi-Status"},
    {208, "Already Reported"},
    {226, "IM Used"},
    {300, "Multiple Choices"},
    {301, "Moved Permanently"},
    {302, "Found"},
    {303, "See Other"},
    {304, "Not Modified"},
    {305, "Use Proxy"},
    {306, "Switch Proxy"},
    {307, "Temporary Redirect"},
    {308, "Permanent Redirect"},
    {400, "Bad Request"},
    {401, "Unauthorized"},
    {402, "Payment Required"},
    {403, "Forbidden"},
    {404, "Not Found"},
    {405, "Method Not Allowed"},
    {406, "Not Acceptable"},
    {407, "Proxy Authentication Required"},
    {408, "Request Timeout"},
    {409, "Conflict"},
    {410, "Gone"},
    {411, "Length Required"},
    {412, "Precondition Failed"},
    {413, "Payload Too Large"},
    {414, "URI Too Long"},
    {415, "Unsupported Media Type"},
    {416, "Range Not Satisfiable"},
    {417, "Expectation Failed"},
    {418, "I'm a teapot"},
    {421, "Misdirected Request"},
    {422, "Unprocessable Entity"},
    {423, "Locked"},
    {424, "Failed Dependency"},
    {425, "Too Early"},
    {426, "Upgrade Required"},
    {428, "Precondition Required"},
    {429, "Too Many Requests"},
    {431, "Request Header Fields Too Large"},
    {451, "Unavailable For Legal Reasons"},
    {500, "Internal Server Error"},
    {501, "Not Implemented"},
    {502, "Bad Gateway"},
    {503, "Service Unavailable"},
    {504, "Gateway Timeout"},
    {505, "HTTP Version Not Supported"},
    {506, "Variant Also Negotiates"},
    {507, "Insufficient Storage"},
    {508, "Loop Detected"},
    {510, "Not Extended"},
    {511, "Network Authentication Required"},
  };

  if (!_messages->contains(code)) {
    throw InvalidArgument()
      << "Invalid status code \"" << code << "\" for HTTP response.";
  }
  return _messages->at(code);
}

}

// -------------------------------------------------------------------------- //
// -------------------------------------------------------------------------- //
//
//             ####   ####   ###  ####    ###   #   #   ###  ####
//             #   #  #     #     #   #  #   #  ##  #  #     #
//             ####   ###    ##   ####   #   #  # # #   ##   ###
//             #   #  #        #  #      #   #  #  ##     #  #
//             #   #  ####  ###   #       ###   #   #  ###   ####
//
// -------------------------------------------------------------------------- //
// -------------------------------------------------------------------------- //

std::string_view HttpResponse::status_message() const {
  if (_status_message.empty()) return default_status_message(_status_code);
  return _status_message;
}

const std::string& HttpResponse::header(std::string_view header_name) const {
  const auto& itr = _headers.find(header_name);
  if (itr == _headers.end()) {
    throw NotFound() << "Header " << header_name << " not found in response.";
  }
  return itr->second;
}

void HttpResponse::header(
  std::string_view header_name,
  std::string_view value
) {
  const auto& itr = _headers.find(header_name);
  if (itr == _headers.end()) {
    _headers.insert(
      std::make_pair(
        std::string{header_name.data(), header_name.size()},
        std::string{value.data(), value.size()}
      )
    );
  } else {
    itr->second.assign(value.begin(), value.end());
  }
}

std::ostream& operator<<(std::ostream& stream, const HttpResponse& res) {
  const char end[] = "\r\n";
  stream
    << "HTTP/1.1 " << res.status() << ' ' << res.status_message() << end;

  for (const auto& [key, value] : res.headers()) {
    stream << key << ": " << value << end;
  }

  if (!res.has_header("Content-Length")) {
    stream << "Content-Length: " << res.body().size() << end;
  }

  // Blank line before the body, nothing after.
  stream << end << res.body();
  return stream;
}

// -------------------------------------------------------------------------- //
// -------------------------------------------------------------------------- //
//
//                  ####    ###   #   # ##### ####   ####
//                  #   #  #   #  #   #   #   #      #   #
//                  ####   #   #  #   #   #   ###    ####
//                  #   #  #   #  #   #   #   #      #   #
//                  #   #   ###    ###    #   #####  #   #
//
// -------------------------------------------------------------------------- //
// -------------------------------------------------------------------------- //

void HttpRouter::attach_routes() {
  for (RouteBase* base_route : get_registered_routes()) {
    auto* route = dynamic_cast<HttpRouteBase*>(base_route);
    LW_CHECK_NULL(route);
    http::MountPath path = http::MountPath::parse_endpoint(route->endpoint());

  }
}

co::Task<void> HttpRouter::run(std::unique_ptr<Socket> conn) {

}

void HttpRouter::mount_route(const http::MountPath& path, HttpRoute* route) {
  PathPart*
  for (const auto& part : path) {
  }
}

}
