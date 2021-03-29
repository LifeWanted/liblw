#include "lw/net/tls_router.h"

#include "gtest/gtest.h"
#include "lw/co/scheduler.h"
#include "lw/co/task.h"
#include "lw/co/testing/destroy_scheduler.h"
#include "lw/io/co/testing/string_connection.h"
#include "lw/net/testing/mock_router.h"
#include "lw/net/testing/tls_credentials.h"
#include "lw/net/tls_options.h"
#include "lw/net/tls.h"

namespace lw::net {
namespace {

using ::lw::io::testing::CoStringConnection;
using ::lw::net::testing::MockRouter;
using ::testing::_;
using ::testing::Return;
using ::testing::StrictMock;

using MockTLSRouter = ::lw::net::TLSRouter<StrictMock<MockRouter>>;

TLSRouterOptions options() {
  return {
    .private_key = testing::KEY_PATH,
    .certificate = testing::CERT_PATH
  };
}

co::Task empty_task() {
  co_await co::next_tick();
}

TEST(TLSRouter, AttachRoutes) {
  MockTLSRouter router{options()};
  EXPECT_CALL(router.base_router(), attach_routes()).Times(1);

  router.attach_routes();
}

TEST(TLSRouter, ConnectionCount) {
  MockTLSRouter router{options()};
  EXPECT_CALL(router.base_router(), connection_count()).WillOnce(Return(42));

  EXPECT_EQ(router.connection_count(), 42);
}

TEST(TLSRouter, Run) {
  MockTLSRouter router{options()};
  EXPECT_CALL(router.base_router(), run(_)).WillOnce(Return(empty_task()));

  auto [client, server] = CoStringConnection::make_connection();
  TLSStreamFactory client_factory{{
    .private_key = testing::KEY_PATH,
    .certificate = testing::CERT_PATH,
    .connection_mode = TLSOptions::CONNECT
  }};
  auto tls_client = client_factory.wrap_stream(std::move(client));

  co::Scheduler::this_thread().schedule(router.run(std::move(server)));
  co::Scheduler::this_thread().schedule([&]() -> co::Task {
    co_await tls_client->handshake();
  });
  co::Scheduler::this_thread().run();
  co::testing::destroy_all_schedulers();
}

}
}
