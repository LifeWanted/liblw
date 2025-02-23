#include "lw/http/client_request.h"

#include <string>
#include <string_view>

#include "lw/err/canonical.h"

namespace lw {
namespace {

constexpr std::string_view HOST_HEADER = "Host";
constexpr std::string_view HEADER_SEP = ": ";
constexpr std::string_view HEADER_ENDL = "\r\n";
constexpr std::string_view CONTENT_LENGTH_HEADER = "Content-Length";
constexpr std::string_view CONTENT_TYPE_HEADER = "Content-Type";

} // namespace

HttpClientRequest::HttpClientRequest(
    std::string_view method, std::string_view host, std::string_view path)
    : _method{method}, _path{path},
      _headers{{std::string(HOST_HEADER), std::string(host)}} {};

std::string_view HttpClientRequest::header(std::string_view header_name) const {
  const auto& itr = _headers.find(header_name);
  if (itr == _headers.end()) {
    throw NotFound() << "Header " << header_name << " not found in request.";
  }
  return itr->second;
}

void HttpClientRequest::header(
    std::string_view header_name, std::string_view value) {
  const auto& itr = _headers.find(header_name);
  if (itr == _headers.end()) {
    _headers.insert(
        std::make_pair(std::string{header_name}, std::string{value}));
  } else {
    itr->second.assign(value.begin(), value.end());
  }
}

Buffer HttpClientRequest::serialize() const {
  io::stream::StringBuffer stream_buffer;
  std::ostream stream{&stream_buffer};
  stream << *this;
  return Buffer{stream_buffer.string().begin(), stream_buffer.string().end()};
}

std::ostream& operator<<(std::ostream& stream, const HttpClientRequest& req) {
  stream << req.method() << " " << req.path() << " HTTP/1.1" << HEADER_ENDL;

  for (const auto& [key, value] : req.headers()) {
    stream << key << HEADER_SEP << value << HEADER_ENDL;
  }

  if (!req.body().empty()) {
    if (!req.has_header(CONTENT_LENGTH_HEADER)) {
      stream << CONTENT_LENGTH_HEADER << HEADER_SEP << req.body().size()
             << HEADER_ENDL;
    }
    if (!req.has_header(CONTENT_TYPE_HEADER)) {
      stream << CONTENT_TYPE_HEADER << HEADER_SEP << req.body().content_type()
             << HEADER_ENDL;
    }
  }

  // Blank line before the body, nothing after.
  stream << HEADER_ENDL << req.body().content();
  return stream;
}

} // namespace lw
