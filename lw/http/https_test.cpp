#include "lw/http/https.h"

#include <memory>
#include <string_view>
#include <tuple>

#include "gtest/gtest.h"
#include "lw/co/future.h"
#include "lw/co/scheduler.h"
#include "lw/co/task.h"
#include "lw/http/http_handler.h"
#include "lw/io/co/testing/string_connection.h"
#include "lw/net/testing/tls_credentials.h"
#include "lw/net/tls.h"
#include "lw/net/tls_options.h"

namespace lw {
namespace {

using ::lw::io::testing::CoStringConnection;

class TestHttpsHandler: public HttpHandler {
public:
  co::Future<void> get() override {
    response().body(request().route_param("endpoint"));
    co_return;
  }
};
LW_REGISTER_HTTP_HANDLER(TestHttpsHandler, "/test/:endpoint");

class HttpsRouterTest: public ::testing::Test {
protected:
  HttpsRouterTest():
    _router{{
      .private_key = net::testing::KEY_PATH,
      .certificate = net::testing::CERT_PATH
    }},
    _factory{{
      .private_key = net::testing::KEY_PATH,
      .certificate = net::testing::CERT_PATH,
      .connection_mode = net::TLSOptions::CONNECT
    }}
  {
    _router.attach_routes();
  }

  std::pair<
    std::unique_ptr<net::TLSStream>,
    std::unique_ptr<CoStringConnection>
  >
  make_connection() {
    std::pair<
      std::unique_ptr<net::TLSStream>,
      std::unique_ptr<CoStringConnection>
    > ret;
    std::unique_ptr<CoStringConnection> client;
    std::tie(client, ret.second) = CoStringConnection::make_connection();
    ret.first = _factory.wrap_stream(std::move(client));
    return ret;
  }

  co::Scheduler& scheduler() const {
    return co::Scheduler::this_thread();
  }

  co::Future<std::string> send_request(
    net::TLSStream& conn,
    std::string_view message
  ) const {
    co_await conn.handshake();

    Buffer buff{1024};
    buff.copy(message.begin(), message.end());
    co_await conn.write({buff.data(), message.size(), /*own_data=*/false});

    std::size_t read_bytes = co_await conn.read(buff);
    std::string response;
    response.reserve(read_bytes);
    response.replace(0, read_bytes, buff);
    co_return response;
  }

  HttpsRouter _router;
  net::TLSStreamFactory _factory;
};

TEST_F(HttpsRouterTest, ExecutesRegisteredHandlers) {
  std::string response;
  auto [client, server] = make_connection();

  scheduler().schedule(_router.run(std::move(server)));
  scheduler().schedule([&]() -> co::Task<void> {
    std::string response = co_await send_request(
      *client,
      "GET /test/foobar HTTP/1.1\r\n"
      "Host: localhost\r\n\r\n"
    );
    EXPECT_EQ(
      response,
      "HTTP/1.1 200 OK\r\n"
      "Content-Length: 6\r\n"
      "\r\n"
      "foobar"
    );
  });

  scheduler().run();
}

TEST_F(HttpsRouterTest, RespondsNotFound) {
  std::string response;
  auto [client, server] = make_connection();

  scheduler().schedule(_router.run(std::move(server)));
  scheduler().schedule([&]() -> co::Task<void> {
    std::string response = co_await send_request(
      *client,
      "GET /gibberish HTTP/1.1\r\n"
      "Host: localhost\r\n\r\n"
    );
    EXPECT_EQ(
      response,
      "HTTP/1.1 404 Not Found\r\n"
      "Content-Type: text/plain\r\n"
      "Content-Length: 10\r\n"
      "\r\n"
      "Not Found."
    );
  });

  scheduler().run();
}

}
}
