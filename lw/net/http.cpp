#include "lw/net/http.h"

#include <cctype>
#include <charconv>
#include <experimental/source_location>
#include <future>
#include <ios>

#include "lw/err/canonical.h"
#include "lw/err/macros.h"

namespace lw::net {
namespace {

using std::experimental::source_location;

/**
 * Returns true if the position given is within the header and is not a
 * whitespace character.
 *
 * @see std::isspace
 */
bool is_not_space(std::string_view header, std::size_t i) {
  return i < header.size() && !std::isspace(header.at(i));
}

/**
 * Asserts that the given position is within the header and is a whitespace
 * character but not a newline.
 *
 * @throws InvalidArgument
 *  If the position given by `i` is outside the header.
 * @throws InvalidArgument
 *  If the character at given position is not whitespace or is either carriage
 *  return (`\r`) or newline (`\n`).
 */
void check_is_space(
  std::string_view header,
  std::size_t i,
  const source_location& loc = source_location::current()
) {
  if (i >= header.size()) {
    throw InvalidArgument(loc)
      << "Unexpected end of input at position " << i << "; expected space.";
  }

  const char c = header.at(i);
  if (!std::isspace(c) || c == '\r' || c == '\n') {
    throw InvalidArgument(loc)
      << "Invalid character in request at position " << i << ". Found 0x"
      << std::hex << (int)c << ", expected space.";
  }
}

/**
 * Asserts that the given position is inside the header and not a newline
 * character.
 *
 * @throws InvalidArgument
 *  If the position given by `i` is outside the header.
 * @throws InvalidArgument
 *  If the character at given position is either carriage return (`\r`) or
 *  newline (`\n`).
 */
void check_not_line_end(
  std::string_view header_view,
  std::size_t i,
  std::string_view expected,
  const source_location& loc = source_location::current()
) {
  if (i >= header_view.size()) {
    throw InvalidArgument(loc)
      << "Unexpected end of input at position " << i << "; expected "
      << expected;
  }

  if (header_view.at(i) == '\r' || header_view.at(i) == '\n') {
    throw InvalidArgument(loc)
      << "Unexpected end of line at position " << i << "; expected "
      << expected;
  }
}

/**
 * Inserts `string_views` defined by the start and end values given as key-value
 * pairs into the provided map.
 */
void insert_view_pairs(
  std::string_view view,
  std::size_t key_start,
  std::size_t key_end,
  std::size_t value_start,
  std::size_t value_end,
  http::KeyValueViewPairs* pairs
) {
  LW_CHECK_NULL(pairs);
  if (key_start >= key_end) return;
  if (value_start >= value_end) {
    pairs->insert({
      view.substr(key_start, key_end - key_start),
      view.substr(value_start, 0)});
  } else {
    pairs->insert({
      view.substr(key_start, key_end - key_start),
      view.substr(value_start, value_end - value_start)
    });
  }
}

/**
 * Parses the string as URL-encoded key-value pairs into the provided map.
 *
 * The values are not copied from the source string.
 */
void parse_query_params(
  std::string_view params_str,
  http::KeyValueViewPairs* query_params
) {
  LW_CHECK_NULL(query_params);

  bool parsing_key = true;
  std::size_t key_start = 0;
  std::size_t key_end = 0;
  std::size_t value_start = 0;
  std::size_t i = 0;

  for (; i < params_str.size(); ++i) {
    const char c = params_str.at(i);
    if (parsing_key) {
      key_end = i;
      value_start = i + 1;
      if (c == '=') {
        parsing_key = false;
      }
    }

    if (c == '&') {
      insert_view_pairs(
        params_str,
        key_start, key_end,
        value_start, i,
        query_params
      );
      key_start = key_end = value_start = i + 1;
      parsing_key = true;
    }
  }

  // Check for a dangling param.
  if (key_start != i) {
    insert_view_pairs(
      params_str,
      key_start, key_end,
      value_start, i,
      query_params
    );
  }
}

std::pair<std::string_view, std::string_view> parse_header_line(
  std::string_view* header_view
) {
  LW_CHECK_NULL(header_view);
  std::size_t i = 0;
  for (; i < header_view->size() && header_view->at(i) != ':'; ++i) {
    check_not_line_end(*header_view, i, "':'");
  }
  if (i >= header_view->size()) {
    throw InvalidArgument() << "Unexpected end of header at position " << i;
  }
  const std::size_t colon_pos = i++;

  for (; i < header_view->size() && std::isspace(header_view->at(i)); ++i) {
    if (header_view->at(i) == '\r' || header_view->at(i) == '\n') break;
  }
  if (i >= header_view->size()) {
    throw InvalidArgument() << "Unexpected end of header at position " << i;
  }
  const std::size_t value_start = i;
  const std::size_t value_end = header_view->find("\r\n", i);
  if (value_end == std::string_view::npos) {
    throw InvalidArgument() << "Unexpected end of input at position " << i;
  }

  auto results = std::make_pair(
    header_view->substr(0, colon_pos),
    header_view->substr(value_start, value_end - value_start)
  );
  header_view->remove_prefix(value_end + 2);
  return results;
}

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
//                 ####   ####   ###   #   #  ####   ###  #####
//                 #   #  #     #   #  #   #  #     #       #
//                 ####   ###   #   #  #   #  ###    ##     #
//                 #   #  #     #  ##  #   #  #        #    #
//                 #   #  ####   ## ##  ###   ####  ###     #
//
// -------------------------------------------------------------------------- //
// -------------------------------------------------------------------------- //

HttpRequest::HttpRequest(std::istream* connection): _connection{connection} {
  LW_CHECK_NULL(connection);
}

std::future<void> HttpRequest::read_header() {
  return std::async(std::launch::async, [this]() -> void {
    if (!_raw_header.empty()) {
      throw FailedPrecondition() << "Header already loaded.";
    }
    for (char c; _connection->get(c);) {
      _raw_header.append(1, c);
      if (_raw_header.ends_with("\r\n\r\n")) break;
      // TODO: Add size limit and slow-connection timeout.
    }
    _raw_header.shrink_to_fit();

    std::size_t method_line_end = _parse_method_line(_raw_header);
    _parse_headers(
      std::string_view{
        _raw_header.data() + method_line_end,
        _raw_header.size() - method_line_end
      }
    );
    _parse_content_length();
  });
}

std::size_t HttpRequest::_parse_method_line(std::string_view header_view) {
  // GET /foo/bar HTTP/1.1\r\n
  // Parse HTTP verb.
  std::size_t i = 0;
  while (is_not_space(header_view, i)) ++i;
  check_is_space(header_view, i);
  _method = header_view.substr(0, i);

  // Parse path and query params.
  std::size_t start = ++i;
  while (is_not_space(header_view, i) && header_view.at(i) != '?') ++i;
  std::size_t end = i;
  if (i < header_view.size() && header_view.at(i) == '?') {
    while (is_not_space(header_view, i)) ++i;
    check_is_space(header_view, i);
    parse_query_params(header_view.substr(end + 1, i - end), &_query_params);
  }
  check_is_space(header_view, i);
  _path = header_view.substr(start, end - start);
  _raw_path = header_view.substr(start, i - start);

  // Parse HTTP version.
  start = ++i;
  while (is_not_space(header_view, i)) ++i;
  if (i + 1 >= header_view.size()) {
    throw InvalidArgument()
      << "Unexpected end of input after HTTP method line.";
  } else if (header_view.at(i) != '\r' || header_view.at(i + 1) != '\n') {
    throw InvalidArgument()
      << "Unexpected characters after HTTP version, expected \\r\\n.";
  }
  _http_version = header_view.substr(start, i - start);

  return i + 2;  // Skip \r\n
}

void HttpRequest::_parse_headers(std::string_view header_view) {
  while (!header_view.starts_with("\r\n")) {
    _headers.insert(parse_header_line(&header_view));
  }
}

void HttpRequest::_parse_content_length() {
  if (!has_header("content-length")) {
    _content_length = 0;
    return;
  }

  std::string_view content_length_str = header("content-length");
  auto res = std::from_chars(
    content_length_str.begin(),
    content_length_str.end(),
    _content_length
  );
  if (res.ptr != content_length_str.end() || _content_length < 0) {
    throw InvalidArgument()
      << "Invalid Content-Length \"" << content_length_str
      << "\"; expected a positive integer.";
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

}
