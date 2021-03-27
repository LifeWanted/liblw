#include "lw/net/tls.h"

#include <memory>
#include <string>

#include "gtest/gtest.h"
#include "lw/co/scheduler.h"
#include "lw/co/testing/destroy_scheduler.h"
#include "lw/io/co/testing/string_connection.h"
#include "lw/io/co/testing/string_stream.h"
#include "lw/net/testing/tls_credentials.h"
#include "lw/net/tls_options.h"

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

// -------------------------------------------------------------------------- //

class TLSStreamTest: public ::testing::Test {
protected:
  TLSStreamTest():
    _client_factory{{
      .private_key = testing::KEY_PATH,
      .certificate = testing::CERT_PATH,
      .connection_mode = TLSOptions::CONNECT
    }},
    _server_factory{{
      .private_key = testing::KEY_PATH,
      .certificate = testing::CERT_PATH,
      .connection_mode = TLSOptions::ACCEPT
    }}
  {
    auto [client, server] = io::testing::CoStringConnection::make_connection();
    _client = _client_factory.wrap_stream(std::move(client));
    _server = _server_factory.wrap_stream(std::move(server));
  }

  ~TLSStreamTest() {
    co::testing::destroy_all_schedulers();
  }

  co::Scheduler& scheduler() const { return co::Scheduler::this_thread(); }

  TLSStreamFactory _client_factory;
  TLSStreamFactory _server_factory;
  std::unique_ptr<TLSStream> _client;
  std::unique_ptr<TLSStream> _server;
};

TEST_F(TLSStreamTest, HandshakesFinish) {
  scheduler().schedule([&]() -> co::Task {
    co_await _client->handshake();
  });
  scheduler().schedule([&]() -> co::Task {
    co_await _server->handshake();
  });
  scheduler().run();
}

TEST_F(TLSStreamTest, CanSendDataFromClientToServer) {
  scheduler().schedule([&]() -> co::Task {
    co_await _client->handshake();
    Buffer buff{6};
    buff.copy("foobar", 6);
    std::size_t written = co_await _client->write(buff);
    EXPECT_EQ(written, 6);
  });
  scheduler().schedule([&]() -> co::Task {
    co_await _server->handshake();
    Buffer buff{10};
    std::size_t read = co_await _server->read(buff);
    EXPECT_EQ(read, 6);
    EXPECT_EQ(static_cast<std::string_view>(buff).substr(0, 6), "foobar");
  });
  scheduler().run();
}

TEST_F(TLSStreamTest, CanSendDataFromServerToClient) {
  scheduler().schedule([&]() -> co::Task {
    co_await _client->handshake();
    Buffer buff{10};
    std::size_t read = co_await _client->read(buff);
    EXPECT_EQ(read, 6);
    EXPECT_EQ(static_cast<std::string_view>(buff).substr(0, 6), "foobar");
  });
  scheduler().schedule([&]() -> co::Task {
    co_await _server->handshake();
    Buffer buff{6};
    buff.copy("foobar", 6);
    std::size_t written = co_await _server->write(buff);
    EXPECT_EQ(written, 6);
  });
  scheduler().run();
}

}
}
