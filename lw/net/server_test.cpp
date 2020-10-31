#include "lw/net/server.h"

#include <chrono>
#include <thread>

#include "gtest/gtest.h"
#include "lw/co/scheduler.h"
#include "lw/co/task.h"
#include "lw/co/testing/destroy_scheduler.h"
#include "lw/io/co/co.h"
#include "lw/net/router.h"
#include "lw/net/socket.h"

namespace lw::net {
namespace {

class TestRouter : public Router {
public:
  void attach_routes() override {
    routes_attached = true;
  }

  co::Task<void> run(std::unique_ptr<io::CoStream> conn) override {
    connection = std::move(conn);
    co_return;
  }

  std::size_t connection_count() const override {
    return connection ? 1 : 0;
  }

  bool routes_attached = false;
  std::unique_ptr<io::CoStream> connection = nullptr;
};

std::jthread run_in_background(Server* server, Router* router) {
  return std::jthread{[=]() {
    server->attach_router(8080, router);
    server->listen();
    server->run();
  }};
}

co::Task<void> connect_and_shutdown(Server* server) {
  Socket sock;
  co_await sock.connect({.hostname = "localhost", .service = "8080"});
  co_await co::next_tick();
  sock.close();
  server->force_close();
}

TEST(Server, RoutersAttachRoutes) {
  TestRouter router;
  Server server;
  EXPECT_FALSE(router.routes_attached);

  server.attach_router(8080, &router);

  EXPECT_TRUE(router.routes_attached);
}

TEST(Server, ServerStartupAndShutdown) {
  TestRouter router;
  Server server;

  std::jthread server_thread = run_in_background(&server, &router);

  while (!server.running()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }

  server.force_close();
  ASSERT_FALSE(server.running());
  server_thread.join();
  co::testing::destroy_all_schedulers();
}

TEST(Server, SendsRequestsToRouter) {
  TestRouter router;
  Server server;
  std::jthread server_thread = run_in_background(&server, &router);

  while (!server.running()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
  }

  co::Scheduler::for_thread(server_thread.get_id()).schedule(
    connect_and_shutdown(&server)
  );
  ASSERT_TRUE(server.running());
  server_thread.join();
  EXPECT_NE(router.connection, nullptr);
  co::testing::destroy_all_schedulers();
}

}
}
