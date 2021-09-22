#include "lw/thread/thread.h"

#include <atomic>
#include <thread>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lw/co/future.h"
#include "lw/co/scheduler.h"
#include "lw/co/task.h"
#include "lw/co/testing/destroy_scheduler.h"

namespace lw {
namespace {

using ::lw::co::testing::destroy_all_schedulers;

TEST(Thread, SpawnsNewThreads) {
  co::Scheduler::this_thread().schedule([&]() -> co::Task {
    co::Promise<std::thread::id> promise;
    co_await thread([&]() -> co::Task {
      promise.set_value(std::this_thread::get_id());
      co_return;
    });

    EXPECT_NE(co_await promise.get_future(), std::this_thread::get_id());
  });

  co::Scheduler::this_thread().run();
  destroy_all_schedulers();
}

}
}
