#include "lw/http/http_handler.h"

#include "lw/co/future.h"
#include "lw/http/http_response.h"

namespace lw {

co::Future<void> HttpHandler::_default_behavior() {
  // TODO(alaina): Determine implemented methods so we can respond with a 405.
  // response.status(HttpResponse::METHOD_NOT_ALLOWED);
  // response.header("Allow", "");
  response().status(HttpResponse::NOT_FOUND);
  return co::make_resolved_future();
}

}
