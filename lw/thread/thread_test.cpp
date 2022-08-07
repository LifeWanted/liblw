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

int test_counter;
void increment() { ++test_counter; }
co::Task increment_task() {
  co_await co::next_tick();
  ++test_counter;
}

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

TEST(Thread, WorksOnVoidFunctors) {
  co::Scheduler::this_thread().schedule([&]() -> co::Task {
    co::Promise<std::thread::id> promise;
    co_await thread([&]() { promise.set_value(std::this_thread::get_id()); });

    EXPECT_NE(co_await promise.get_future(), std::this_thread::get_id());
  });

  co::Scheduler::this_thread().run();
  destroy_all_schedulers();
}

TEST(Thread, WorksOnOrdinaryVoidFunctions) {
  co::Scheduler::this_thread().schedule([&]() -> co::Task {
    test_counter = 0;
    co_await thread(&increment);
    EXPECT_EQ(test_counter, 1);
  });

  co::Scheduler::this_thread().run();
  destroy_all_schedulers();
}

TEST(Thread, WorksOnOrdinaryFutureFunctions) {
  co::Scheduler::this_thread().schedule([&]() -> co::Task {
    test_counter = 0;
    co_await thread(&increment_task);
    EXPECT_EQ(test_counter, 1);
  });

  co::Scheduler::this_thread().run();
  destroy_all_schedulers();
}

}
}
