#include "lw/http/http.h"

#include <exception>
#include <memory>

#include "lw/co/future.h"
#include "lw/co/task.h"
#include "lw/err/canonical.h"
#include "lw/io/co/co.h"
#include "lw/http/internal/http_mount_path.h"
#include "lw/http/http_request.h"
#include "lw/log/log.h"

namespace lw {
namespace {

using ::lw::http::internal::EndpointTrie;

void respond_failure(HttpResponse& res, int status, std::string_view body) {
  res.status(status);
  res.header("Content-Type", "text/plain");
  res.body(std::string{body});
}

co::Future<bool> try_read_header(HttpRequest& req, HttpResponse& res) {
  try {
    co_await req.read_header();
    co_return true;
  } catch (const InvalidArgument& err) {
    log(INFO) << "Malformed header from client: " << err.what();
    respond_failure(res, HttpResponse::BAD_REQUEST, err.what());
    co_return false;
  }
}

co::Future<void> finish_request(
  io::CoStream& conn,
  HttpRequest& req,
  HttpResponse& res
) {
  log(INFO)
    << "Responding " << res.status() << " to " << req.method() << ' '
    << req.path();
  co_await conn.write(res.serialize());

  if (
    !req.has_header("connection") || req.header("connection") != "keep-alive"
  ) {
    conn.close();
  }
}

co::Future<void> run_request(io::CoStream& conn, EndpointTrie& trie) {
  io::CoReader reader{conn};
  HttpRequest request{reader};
  HttpResponse response;

  if (!co_await try_read_header(request, response)) {
    co_await finish_request(conn, request, response);
    co_return;
  }

  auto match_results = trie.match(request.path());
  if (!match_results) {
    respond_failure(response, HttpResponse::NOT_FOUND, "Not Found.");
    co_await finish_request(conn, request, response);
    co_return;
  }

  request.route_params(std::move(match_results->parameters));
  auto handler = match_results->endpoint.make_handler(request, response);
  log(INFO)
    << "Running handler for " << request.method() << ' '
    << match_results->endpoint.route();

  // TODO(alaina): Introduce HttpStatus error class for use by HttpHandlers,
  // then wrap this invocation in a try-catch for that type and respond with an
  // appropriate HTTP error message.
  if (request.method() == "DELETE")       co_await handler->del();
  else if (request.method() == "GET")     co_await handler->get();
  else if (request.method() == "HEAD")    co_await handler->head();
  else if (request.method() == "OPTIONS") co_await handler->options();
  else if (request.method() == "PATCH")   co_await handler->patch();
  else if (request.method() == "POST")    co_await handler->post();
  else if (request.method() == "PUT")     co_await handler->put();
  else {
    respond_failure(response, HttpResponse::BAD_REQUEST, "Unknown method.");
  }

  co_await finish_request(conn, request, response);
}

}

void HttpRouter::attach_routes() {
  for (net::RouteBase* route_base : get_registered_routes()) {
    auto* route = static_cast<HttpRoute*>(route_base);
    log(INFO) << "Attaching HttpRoute " << route->factory().route();
    _trie.insert(
      http::internal::MountPath::parse_endpoint(route->factory().route()),
      route->factory()
    );
  }
}

co::Task<void> HttpRouter::run(std::unique_ptr<io::CoStream> conn) {
  ++_connection_counter;
  log(INFO)
    << "HttpRouter handling " << _connection_counter << " concurrent requests.";

  while (conn->good()) {
    try {
      co_await run_request(*conn, _trie);
    } catch(const Error& err) {
      if (conn->good()) conn->close();
      log(ERROR) << "Unhandled application error: " << err.what();
    }
  }
  --_connection_counter;
}

}
