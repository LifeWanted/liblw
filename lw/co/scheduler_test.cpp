#include "lw/co/scheduler.h"

#include <sys/timerfd.h>

#include "gtest/gtest.h"
#include "lw/co/events.h"
#include "lw/co/testing/destroy_scheduler.h"


namespace lw::co {
namespace {

struct AwaitableObserver {
  bool ready_called = false;
  bool suspend_called = false;
  bool resume_called = false;
};

class TestAwaitable {
public:
  TestAwaitable(AwaitableObserver& observer): _observer{observer} {}

  bool await_ready() {
    _observer.ready_called = true;
    return false;
  }

  void await_suspend() {
    _observer.suspend_called = true;
  }

  void await_resume() {
    _observer.resume_called = true;
  }

private:
  AwaitableObserver& _observer;
};

// -------------------------------------------------------------------------- //

class SchedulerTest : public ::testing::Test {
protected:
  SchedulerTest() {}
  ~SchedulerTest() {
    testing::destroy_this_scheduler();
  }
};

TEST_F(SchedulerTest, ScheduleAndRun) {
  int timer = ::timerfd_create(CLOCK_REALTIME, TFD_NONBLOCK);
  ASSERT_GT(timer, 0) << "Timer file descripter.";
  ::itimerspec spec{
    .it_interval = {0},
    .it_value = {.tv_sec = 0, .tv_nsec = 15 * 1000000} // 15ms
  };
  ASSERT_EQ(::timerfd_settime(timer, 0, &spec, nullptr), 0);

  AwaitableObserver observer;
  TestAwaitable awaitable{observer};

  Scheduler::this_thread()
    .schedule(timer, Event::READABLE | Event::ONE_SHOT, std::move(awaitable));

  EXPECT_FALSE(observer.resume_called);
  Scheduler::this_thread().run();
  EXPECT_TRUE(observer.resume_called);
}

}
}
