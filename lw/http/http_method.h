#pragma once

#include <string_view>

namespace lw {

enum class HttpMethod {
  GET,
  HEAD,
  OPTIONS,
  TRACE,
  PUT,
  DELETE,
  POST,
  PATCH,
  CONNECT
};

std::string_view to_string(HttpMethod method);

} // namespace lw
