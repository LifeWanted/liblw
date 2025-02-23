#include "lw/http/http_body.h"

#include <string_view>

namespace lw {
namespace {

constexpr std::string_view PLAIN_TEXT_CONTENT_TYPE = "text/plain";

}

HttpBody HttpBody::text(std::string_view content) {
  return HttpBody(PLAIN_TEXT_CONTENT_TYPE, content);
}

} // namespace lw
