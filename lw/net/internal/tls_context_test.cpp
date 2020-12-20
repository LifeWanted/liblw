#include "lw/net/internal/tls_context.h"

#include <memory>

#include "gtest/gtest.h"
#include "lw/net/tls_options.h"
#include "lw/net/testing/tls_credentials.h"

namespace lw::net::internal {
namespace {

TEST(TLSContextImpl, MakesClients) {
  std::unique_ptr<TLSContextImpl> context = TLSContextImpl::from_options({
    .private_key = {testing::KEY_PATH},
    .certificate = {testing::CERT_PATH}
  });

  std::unique_ptr<TLSClientImpl> client;
  EXPECT_NO_THROW(client = context->make_client());
  EXPECT_NE(client, nullptr);
}

}
}
