#pragma once

#include <memory>

#include "lw/net/internal/tls_client.h"
#include "lw/net/tls_options.h"
#include "openssl/base.h"

namespace lw::net::internal {

class TLSContextImpl {
public:
  ~TLSContextImpl();

  static std::unique_ptr<TLSContextImpl>
  from_options(const TLSOptions& options);

  std::unique_ptr<TLSClientImpl> make_client();

private:
  TLSContextImpl(const TLSOptions& options, SSL_CTX* context)
      : _connection_mode{options.connection_mode}, _context{context} {}

  TLSOptions::ConnectionMode _connection_mode;
  SSL_CTX* _context = nullptr;
};

} // namespace lw::net::internal
