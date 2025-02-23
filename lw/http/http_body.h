#pragma once

#include <string>
#include <string_view>

namespace lw {

// TODO: #16 - Add streaming and additional mime types.
class HttpBody {
public:
  HttpBody() = default;
  HttpBody(const HttpBody&) = default;
  HttpBody& operator=(const HttpBody&) = default;
  HttpBody(HttpBody&&) = default;
  HttpBody& operator=(HttpBody&&) = default;
  ~HttpBody() = default;

  HttpBody(std::string_view content_type, std::string_view content)
      : _content_type{content_type}, _content{content} {}

  static HttpBody text(std::string_view content);

  bool empty() const { return _content.empty(); }
  std::size_t size() const { return _content.size(); }
  std::size_t content_length() const { return size(); }
  std::string_view content_type() const { return _content_type; }
  std::string content() const { return _content; }

private:
  std::string _content_type;
  std::string _content;
};

} // namespace lw
