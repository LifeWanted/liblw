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

  std::future<void> run(Socket* conn) override {
    connection = conn;
    return co::make_future();
  }

  bool routes_attached = false;
  Socket* connection = nullptr;
};

TEST(Server, RoutersAttachRoutes) {
  TestRouter router;
  Server server;
  EXPECT_FALSE(router.routes_attached);

  server.attach_router(8080, &router);

  EXPECT_TRUE(router.routes_attached);
}

TEST(Server, SendsRequestsToRouter) {

}

}
}
