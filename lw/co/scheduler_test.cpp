#include "lw/co/scheduler.h"

#include <sys/timerfd.h>

#include "gtest/gtest.h"
#include "lw/co/events.h"
#include "lw/co/testing/destroy_scheduler.h"

namespace lw::co {
namespace {

class SchedulerTest : public ::testing::Test {
protected:
  SchedulerTest() {}
  ~SchedulerTest() {
    testing::destroy_all_schedulers();
  }

  Task<void> test_task(int& counter) {
    ++counter;
    co_await Scheduler::this_thread().next_tick();
    ++counter;
  }

  template <typename Func>
  Task<void> observe(Func&& f) {
    f();
    co_return;
  }

  template <typename Func>
  Task<void> observe_next_tick(Func&& f) {
    co_await Scheduler::this_thread().next_tick();
    f();
  }
};

TEST_F(SchedulerTest, RunATask) {
  int counter = 0;
  {
    auto task = test_task(counter);
    EXPECT_EQ(counter, 0);
    Scheduler::this_thread().schedule(std::move(task));
  }
  Scheduler::this_thread().run();
  EXPECT_EQ(counter, 2);
}

TEST_F(SchedulerTest, RunMultipleTasks) {
  int counter = 0;
  {
    auto task = test_task(counter);
    EXPECT_EQ(counter, 0);
    Scheduler::this_thread().schedule(std::move(task));
  }
  Scheduler::this_thread().schedule(observe([&]() {
    // This should execute when `test_task` suspends for next tick.
    EXPECT_EQ(counter, 1);
    ++counter;
  }));
  Scheduler::this_thread().schedule(observe_next_tick([&]() {
    // This should execute after `test_task` resumes its next tick.
    EXPECT_EQ(counter, 3);
    ++counter;
  }));
  Scheduler::this_thread().run();
  EXPECT_EQ(counter, 4);
}

}
}
