#include "lw/co/systems/epoll.h"

#include <chrono>
#include <sys/timerfd.h>

#include "gtest/gtest.h"
#include "lw/co/events.h"

namespace lw::co::internal {
namespace {

using std::chrono::high_resolution_clock;
using std::chrono::milliseconds;

TEST(EPoll, EmptyTryWaitShouldNotBlock) {
  EPoll epoll;
  auto start = high_resolution_clock::now();
  EXPECT_EQ(epoll.try_wait(), 0);
  auto end = high_resolution_clock::now();

  EXPECT_LT(end - start, milliseconds(5));
}

TEST(EPoll, TimerFd) {
  int timer = ::timerfd_create(CLOCK_REALTIME, TFD_NONBLOCK);
  ASSERT_GT(timer, 0) << "Timer file descripter.";
  ::itimerspec spec{
    .it_interval = {0},
    .it_value = {.tv_sec = 0, .tv_nsec = 15 * 1000000} // 15ms
  };
  auto start = high_resolution_clock::now();
  ASSERT_EQ(::timerfd_settime(timer, 0, &spec, nullptr), 0);

  EPoll epoll;
  bool called = false;
  epoll.add(timer, Event::READABLE, [&]() { called = true; });

  EXPECT_FALSE(called);
  epoll.try_wait();
  ASSERT_LT(high_resolution_clock::now() - start, milliseconds(15));
  EXPECT_FALSE(called);
  epoll.wait_for(milliseconds(5));
  ASSERT_LT(high_resolution_clock::now() - start, milliseconds(15));
  EXPECT_FALSE(called);
  epoll.wait();
  EXPECT_GE(high_resolution_clock::now() - start, milliseconds(15));
  EXPECT_TRUE(called);
}

}
}
