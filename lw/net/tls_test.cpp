#include "lw/net/tls.h"

#include <memory>
#include <string>

#include "gtest/gtest.h"
#include "lw/io/co/testing/string_stream.h"
#include "lw/net/testing/tls_credentials.h"

namespace lw::net {
namespace {

TEST(TLS, CanConstructFactory) {
  TLSStreamFactory factory{{
    .private_key = testing::KEY_PATH,
    .certificate = testing::CERT_PATH
  }};
}

TEST(TLS, FactoryCanWrapCoStreams) {
  std::string out;
  auto stream = std::make_unique<io::testing::CoStringStream>("", out);
  TLSStreamFactory factory{{
    .private_key = testing::KEY_PATH,
    .certificate = testing::CERT_PATH
  }};

  std::unique_ptr<TLSStream> wrapped = factory.wrap_stream(std::move(stream));
  EXPECT_NE(wrapped, nullptr);
}

}
}
