#include "lw/co/scheduler.h"

#include <chrono>
#include <sys/timerfd.h>

#include "gtest/gtest.h"
#include "lw/co/events.h"
#include "lw/co/task.h"
#include "lw/co/testing/destroy_scheduler.h"

namespace lw::co {
namespace {

using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;

class SchedulerTest : public ::testing::Test {
protected:
  SchedulerTest() {}
  ~SchedulerTest() noexcept {
    testing::destroy_all_schedulers();
  }

  int create_timer(std::chrono::nanoseconds delay) {
    int timer = ::timerfd_create(CLOCK_REALTIME, TFD_NONBLOCK);
    auto seconds = duration_cast<std::chrono::seconds>(delay);
    ::itimerspec spec{
      .it_interval = {0},
      .it_value = {
        .tv_sec = seconds.count(),
        .tv_nsec =
          duration_cast<std::chrono::nanoseconds>(delay - seconds).count()
      }
    };
    ::timerfd_settime(timer, /*flags=*/0, &spec, nullptr);
    return timer;
  }
};

TEST_F(SchedulerTest, NextTick) {
  int result = 0;
  auto co = [&](int expect) -> Task<void> {
    EXPECT_EQ(result, expect);
    ++result;
    co_await next_tick();
    EXPECT_EQ(result, expect + 2);
    ++result;
  };
  Scheduler::this_thread().schedule(co(0));
  Scheduler::this_thread().schedule(co(1));
  EXPECT_EQ(result, 0);
  Scheduler::this_thread().run();
  EXPECT_EQ(result, 4);
}

TEST_F(SchedulerTest, FDEvents) {
  auto sleep = std::chrono::milliseconds(15);
  Scheduler::this_thread().schedule([&]() -> Task<void> {
    int handle = create_timer(sleep);
    co_await fd_readable(handle);
    ::close(handle);
  });
  auto start = high_resolution_clock::now();
  Scheduler::this_thread().run();
  auto end = high_resolution_clock::now();
  EXPECT_GT(end, start + sleep);
}

}
}
