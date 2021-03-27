#include "lw/co/scheduler.h"

#include <atomic>
#include <chrono>
#include <sys/timerfd.h>
#include <thread>

#include "gtest/gtest.h"
#include "lw/co/events.h"
#include "lw/co/task.h"
#include "lw/co/testing/destroy_scheduler.h"

namespace lw::co {
namespace {

using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;

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

class SchedulerTest : public ::testing::Test {
protected:
  SchedulerTest() {}
  ~SchedulerTest() noexcept {
    testing::destroy_all_schedulers();
  }
};

TEST_F(SchedulerTest, StartAndStop) {
  std::atomic_int ticks = 0;
  std::atomic_int* ticks_ptr = &ticks;
  std::jthread scheduler_thread{[ticks_ptr]() {
    // Schedule an infinite task that will keep the scheduler running.
    Scheduler::this_thread().schedule([ticks_ptr]() -> Task {
      while (true) {
        ++(*ticks_ptr);
        int timer = create_timer(std::chrono::milliseconds(1));
        co_await fd_readable(timer);
        ::close(timer);
      }
    });

    Scheduler::this_thread().run();
  }};

  // Make sure the scheduler gets going and doesn't stop on its own.
  while (ticks.load() < 5) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }

  // Now kill it.
  Scheduler::for_thread(scheduler_thread.get_id()).stop();
  scheduler_thread.join();
}

TEST_F(SchedulerTest, NextTick) {
  int result = 0;
  auto co = [&](int expect) -> Task {
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
  Scheduler::this_thread().schedule([&]() -> Task {
    int handle = create_timer(sleep);
    co_await fd_readable(handle);
    ::close(handle);
  });
  auto start = high_resolution_clock::now();
  Scheduler::this_thread().run();
  auto end = high_resolution_clock::now();
  EXPECT_GT(end, start + sleep);
}

TEST_F(SchedulerTest, TaskCallback) {
  int callback_called = 0;
  Scheduler::this_thread().schedule(
    []() -> Task {
      int handle = create_timer(std::chrono::milliseconds(1));
      co_await fd_readable(handle);
      ::close(handle);
    },
    [&]() { ++callback_called; }
  );
  EXPECT_EQ(callback_called, 0);
  Scheduler::this_thread().run();
  EXPECT_EQ(callback_called, 1);
}

}
}
