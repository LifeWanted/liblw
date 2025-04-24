#include "lw/http/http_handler.h"

#include "lw/co/future.h"
#include "lw/http/http_response.h"
#include "lw/http/http_status.h"

namespace lw {

co::future<void> HttpHandler::_default_behavior() {
  // TODO(alaina): Determine implemented methods so we can respond with a 405.
  // response.status(HttpStatus::METHOD_NOT_ALLOWED);
  // response.header("Allow", "");
  response().status(HttpStatus::NOT_FOUND);
  return co::make_resolved_future();
}

} // namespace lw
