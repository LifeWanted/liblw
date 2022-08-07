#include "lw/grpc/internal/service_wrapper.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lw/co/future.h"
#include "lw/co/scheduler.h"
#include "lw/co/task.h"
#include "lw/co/testing/destroy_scheduler.h"

namespace lw::grpc::internal {
namespace {

using ::lw::co::testing::destroy_all_schedulers;

TEST(ServiceWrapper, CallsAddedRunners) {
  ServiceWrapper wrapper{nullptr};
  int counter = 0;
  wrapper.add_runner([&](auto*) -> co::Future<void> {
    EXPECT_EQ(++counter, 1);
    co_await co::next_tick();
    EXPECT_EQ(counter, 2);
  });
  wrapper.add_runner([&](auto*) -> co::Future<void> {
    EXPECT_EQ(++counter, 2);
    co_await co::next_tick();
    EXPECT_EQ(counter, 2);
  });

  co::Scheduler::this_thread().schedule([&]() -> co::Task {
    co_await wrapper.initialize(
      // Don't think about it.
      reinterpret_cast<::grpc::ServerCompletionQueue*>(&counter)
    );
    EXPECT_EQ(counter, 2);
  });
  co::Scheduler::this_thread().run();
  destroy_all_schedulers();
}

}
}
