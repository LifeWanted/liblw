#include "lw/http/rest/rest_router.h"

#include "lw/io/co/co.h"
#include "lw/http/http_request.h"
#include "lw/http/http_response.h"
#include "lw/http/rest/rest_request.h"
#include "lw/http/rest/rest_response.h"
#include "lw/http/rest/rest_handler.h"
#include "lw/log/log.h"

namespace lw {
namespace {

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

co::Future<void> finish_http_request(
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

co::Future<void> run_request(
  io::CoStream& conn,
  EndpointTrie<BaseRestHandlerFactory>& trie
) {
  io::CoReader reader{conn};
  HttpRequest http_request{reader};
  HttpResponse http_response;

  if (!co_await try_read_header(http_request, http_response)) {
    co_await finish_http_request(conn, http_request, http_response);
    co_return;
  }

  auto match_results = trie.match(request.path());
  if (!match_results) {
    respond_failure(http_response, HttpResponse::NOT_FOUND, "Not Found.");
    co_await finish_http_request(conn, http_request, http_response);
    co_return;
  }

  http_request.route_params(std::move(match_results->parameters));
  RestRequest request{std::move(http_request)};
  RestResponse response{request, std::move(http_response)};
  auto handler = match_results->endpoint.make_handler(request, response);
  log(INFO)
    << "Running handler for " << request.method() << ' '
    << match_results->endpoint.route();

  co_await handler->pre_body();
  co_await request.parse_body();
  co_await handler->pre_run();
  co_await handler->run();
  co_await handler->post_run();

  co_await finish_request(conn, request, response);
}

}

void RestRouter::attach_routes() {
  for (net::RouteBase* route_base : get_registered_routes()) {
    auto* route = static_cast<RestRoute*>(route_base);
    log(INFO) << "Attaching RestRoute " << route->factory().route();
    _trie.insert(
      http::internal::MountPath::parse_endpoint(route->factory().route()),
      route->factory()
    );
  }
}

co::Task RestRouter::run(std::unique_ptr<io::CoStream> conn) {
  ++_connection_counter;
  log(INFO)
    << "RestRouter handling " << _connection_counter << " concurrent requests.";

  while (conn->good()) {
    try {
      co_await run_once(*conn);
    } catch(const Error& err) {
      if (conn->good()) conn->close();
      log(ERROR) << "Unhandled application error: " << err.what();
    }
  }
  --_connection_counter;
}

co::Future<void> RestRouter::run_once(io::CoStream& conn) {
  return run_request(conn, _trie);
}

}

}
