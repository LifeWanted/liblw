#include "lw/net/router.h"

#include <future>

#include "gtest/gtest.h"

namespace lw::net {
namespace {

class TestRouter: public Router {
public:
  void attach_routes() override {};
  std::future<void> run(Socket* conn) override {
    std::promise<void> p;
    return p.get_future();
  };

  const std::unordered_set<RouteBase*>& test_get_registered_routes() {
    return get_registered_routes();
  }
};

class TestRoute: public RouteBase {};

TEST(Router, RoutesAreRegisterable) {
  TestRoute route;
  register_route<TestRouter>(&route);

  TestRouter router;
  const auto& routes = router.test_get_registered_routes();
  ASSERT_EQ(routes.size(), 1);
  EXPECT_TRUE(routes.contains(&route));
}

}
}
