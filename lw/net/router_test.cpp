#include "lw/net/router.h"

#include <memory>

#include "gtest/gtest.h"
#include "lw/co/task.h"
#include "lw/io/co/co.h"

namespace lw::net {
namespace {

class TestRouter: public Router {
public:
  void attach_routes() override {};
  co::Task<void> run(std::unique_ptr<io::CoStream> conn) override {
    co_return;
  };
  std::size_t connection_count() const override { return 0; }

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
