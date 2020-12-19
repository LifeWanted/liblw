#pragma once

#include <memory>

#include "lw/net/internal/tls_client.h"
#include "lw/net/tls_options.h"
#include "openssl/types.h"

namespace lw::net::internal {

class TLSContextImpl {
public:
  ~TLSContextImpl();

  static std::unique_ptr<TLSContextImpl> from_options(
    const TLSOptions& options
  );

  std::unique_ptr<TLSClientImpl> make_client();

private:
  TLSContextImpl(SSL_CTX* context): _context{context} {}

  SSL_CTX* _context = nullptr;
};

}
