#include "lw/net/server.h"

#include <future>

#include "gtest/gtest.h"
#include "lw/co/future.h"
#include "lw/net/router.h"
#include "lw/net/socket.h"

namespace lw::net {
namespace {

class TestRouter : public Router {
public:
  void attach_routes() override {
    routes_attached = true;
  }

  std::future<void> run(std::unique_ptr<Socket> conn) override {
    connection = std::move(conn);
    return co::make_future();
  }

  std::size_t connection_count() const override {
    return connection ? 1 : 0;
  }

  bool routes_attached = false;
  std::unique_ptr<Socket> connection = nullptr;
};

TEST(Server, RoutersAttachRoutes) {
  TestRouter router;
  Server server;
  EXPECT_FALSE(router.routes_attached);

  server.attach_router(8080, &router);

  EXPECT_TRUE(router.routes_attached);
}

TEST(Server, SendsRequestsToRouter) {
  TestRouter router;
  Server server;
  server.attach_router(8080, &router);
  server.listen().get();
  auto running = server.run();

  Socket sock;
  sock.connect({.hostname = "localhost", .service = "8080"}).get();
  EXPECT_NE(router.connection, nullptr);
  sock.close();
  server.force_close();
  running.get();
}

}
}
