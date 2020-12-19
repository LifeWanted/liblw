#include "lw/net/tls.h"

#include <string_view>

#include "gtest/gtest.h"

namespace lw::net {
namespace {

constexpr std::string_view CERT_PATH = "lw/net/testing/test.cert.pem";
constexpr std::string_view KEY_PATH = "lw/net/testing/test.key.pem";

TEST(TLS, CanConstructFactory) {
  TLSStreamFactory factory{{
    .private_key = KEY_PATH,
    .certificate = CERT_PATH
  }};
}

}
}
