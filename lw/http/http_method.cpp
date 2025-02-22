#include "lw/http/http_method.h"

#include <string_view>
#include <unordered_map>

#include "lw/err/canonical.h"

namespace lw {

std::string_view to_string(HttpMethod method) {
  static std::unordered_map<HttpMethod, std::string_view> method_names{
      {HttpMethod::GET, "GET"},
      {HttpMethod::HEAD, "HEAD"},
      {HttpMethod::OPTIONS, "OPTIONS"},
      {HttpMethod::TRACE, "TRACE"},
      {HttpMethod::PUT, "PUT"},
      {HttpMethod::DELETE, "DELETE"},
      {HttpMethod::POST, "POST"},
      {HttpMethod::PATCH, "PATCH"},
      {HttpMethod::CONNECT, "CONNECT"},
  };
  auto itr = method_names.find(method);
  if (itr == method_names.end()) {
    throw InvalidArgument()
        << "Unknown HttpMethod " << static_cast<int>(method);
  }
  return itr->second;
}

} // namespace lw
