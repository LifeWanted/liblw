#pragma once

#include "gmock/gmock.h"
#include "lw/net/router.h"

namespace lw::net::testing {

class MockRouter: public Router {
public:
  MOCK_METHOD(void, attach_routes, (), (override));
  MOCK_METHOD(co::Task, run, (std::unique_ptr<io::CoStream> conn), (override));
  MOCK_METHOD(std::size_t, connection_count, (), (const override));
};

}
