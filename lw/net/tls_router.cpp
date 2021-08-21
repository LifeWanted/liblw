#include "lw/net/tls_router.h"

#include "lw/co/future.h"
#include "lw/co/task.h"
#include "lw/log/log.h"
#include "lw/net/router.h"
#include "lw/net/tls.h"
#include "lw/net/tls_options.h"

namespace lw::net::internal {

co::Future<std::unique_ptr<net::TLSStream>> tls_wrap_connection(
  net::TLSStreamFactory& tls_factory,
  std::unique_ptr<io::CoStream> conn
) {
  std::unique_ptr<net::TLSStream> tls_conn;
  try {
    tls_conn = tls_factory.wrap_stream(std::move(conn));
    co_await tls_conn->handshake();
  } catch(const Error& err) {
    if (tls_conn->good()) tls_conn->close();
    log(INFO) << "Failed to establish TLS connection/handshake: " << err.what();
  }
  co_return tls_conn;
}

}
