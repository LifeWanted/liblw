#include "lw/http/http_request.h"

#include <charconv>
#include <experimental/source_location>
#include <istream>
#include <string_view>

#include "lw/co/future.h"
#include "lw/err/canonical.h"
#include "lw/err/macros.h"
#include "lw/http/headers.h"
#include "lw/io/co/co.h"

namespace lw {
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
 * @throw InvalidArgument
 *  If the position given by `i` is outside the header.
 * @throw InvalidArgument
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
 * @throw InvalidArgument
 *  If the position given by `i` is outside the header.
 * @throw InvalidArgument
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
  http::HeadersView* pairs
) {
  LW_CHECK_NULL(pairs);
  if (key_start >= key_end) return;
  if (value_start >= value_end) {
    pairs->insert({
      view.substr(key_start, key_end - key_start),
      view.substr(value_start, 0)
    });
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
  http::HeadersView* query_params
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
      if (c == '=') parsing_key = false;
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

/**
 * Parses the line as an HTTP header, returning the Key-Value pair and adjusting
 * the header view to after the line.
 *
 * @throw InvalidArgument
 *  If the end of the input is found before a proper header line is parsed.
 *
 * @param header_view
 *  A point to a string view containing an HTTP header. This view will be
 *  trimmed to exclude the parsed header line.
 *
 * @return
 *  The Key-Value pair parsed from the line of header.
 */
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

  // TODO(alaina): Trim the right-end of the header value. It could containe
  // extra spaces before the \r\n which should be ignored.

  auto results = std::make_pair(
    header_view->substr(0, colon_pos),
    header_view->substr(value_start, value_end - value_start)
  );
  header_view->remove_prefix(value_end + 2);
  return results;
}

}

co::Future<void> HttpRequest::read_header() {
  if (!_raw_header.empty()) {
    throw FailedPrecondition() << "Header already loaded.";
  }
  _raw_header = std::string{co_await _connection.read_until("\r\n\r\n")};
  _raw_header.shrink_to_fit();

  // TODO(alaina): Check that a proper header was loaded. If "\r\n\r\n" is never
  // encountered, that co_await will never return. This should schedule a
  // timeout to close the connection if no data is received in an appropriate
  // amount of time. If the connection closes before the header termination is
  // received then an empty buffer is returned.

  std::size_t method_line_end = _parse_method_line(_raw_header);
  _parse_headers(
    std::string_view{
      _raw_header.data() + method_line_end,
      _raw_header.size() - method_line_end
    }
  );
  _parse_content_length();
}

co::Future<Buffer> HttpRequest::body() const {
  // TODO(alaina): Implement other methods for determining the end of an HTTP
  // request's body.
  if (content_length() == 0) return co::make_resolved_future(Buffer{});
  if (content_length() > 0) return _connection.read(content_length());

  throw Internal()
    << "Unsupported method of content body determination in request.";
}

std::size_t HttpRequest::_parse_method_line(std::string_view header_view) {
  // GET /foo/bar HTTP/1.1\r\n
  // Parse HTTP verb.
  std::size_t i = 0;
  while (is_not_space(header_view, i)) ++i;
  check_is_space(header_view, i); // May be end of sequence.
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

}
