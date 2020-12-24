#include "lw/http/https.h"

#include "lw/log/log.h"
#include "lw/net/tls.h"
#include "lw/net/tls_options.h"

namespace lw {

HttpsRouter::HttpsRouter(const Options& options):
  _tls{{
    .private_key = options.private_key,
    .certificate = options.certificate,
    .connection_mode = net::TLSOptions::ACCEPT
  }}
{}

co::Task<void> HttpsRouter::run(std::unique_ptr<io::CoStream> conn) {
  ++_connection_counter;
  log(INFO)
    << "HttpsRouter handling " << _connection_counter
    << " concurrent requests.";

  std::unique_ptr<net::TLSStream> tls_conn;
  try {
    tls_conn = _tls.wrap_stream(std::move(conn));
    co_await tls_conn->handshake();
  } catch(const Error& err) {
    if (tls_conn->good()) tls_conn->close();
    log(INFO) << "Failed to establish TLS connection/handshake: " << err.what();
  }

  if (tls_conn) {
    while (tls_conn->good()) {
      try {
        co_await _http_router.run_once(*tls_conn);
      } catch(const Error& err) {
        if (tls_conn->good()) tls_conn->close();
        log(ERROR) << "Unhandled application error: " << err.what();
      }
    }
  }
  --_connection_counter;
}

}
