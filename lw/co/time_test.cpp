#include "lw/co/time.h"

#include <chrono>
#include <thread>

#include "gtest/gtest.h"
#include "lw/co/task.h"

namespace lw::co {
namespace {

using ::std::chrono::duration_cast;
using ::std::chrono::milliseconds;
using ::std::chrono::steady_clock;

template <typename TimePoint>
Task<void> test_sleep_until(const TimePoint& time_point) {
  co_await sleep_until(time_point);
}

template <typename Rep, typename Period>
Task<void> test_sleep_for(const std::chrono::duration<Rep, Period>& duration) {
  co_await sleep_for(duration);
}

template <typename Rep, typename Period>
Task<void> test_observe_sleep_for(
  int& counter,
  const std::chrono::duration<Rep, Period>& duration
) {
  ++counter;
  co_await sleep_for(duration);
  ++counter;
  co_await sleep_for(duration);
  ++counter;
}

TEST(Sleeping, SleepUntil) {
  auto duration = milliseconds(5);
  auto before = steady_clock::now();
  test_sleep_until(before + duration).wait().get();
  auto after = steady_clock::now();

  EXPECT_GE(after, before + duration)
    << duration_cast<milliseconds>(after.time_since_epoch()).count() << " vs "
    << duration_cast<milliseconds>(
      (before + duration).time_since_epoch()
    ).count();
}

TEST(Sleeping, SleepFor) {
  auto duration = milliseconds(5);
  auto before = steady_clock::now();
  test_sleep_for(duration).wait().get();
  auto after = steady_clock::now();

  EXPECT_GE(after, before + duration);
}

TEST(Sleeping, ObserveSleep) {
  int counter = 0;
  auto duration = milliseconds(10);
  auto before = steady_clock::now();
  Task<void> task = test_observe_sleep_for(counter, duration);
  EXPECT_EQ(counter, 0);
  EXPECT_FALSE(task.done());
  EXPECT_TRUE(task.ready());

  EXPECT_TRUE(task.resume());
  EXPECT_EQ(counter, 1);
  EXPECT_FALSE(task.done());
  EXPECT_FALSE(task.ready());

  std::this_thread::sleep_for(duration);
  EXPECT_GE(steady_clock::now(), before + duration);
  EXPECT_EQ(counter, 1);
  EXPECT_FALSE(task.done());
  EXPECT_TRUE(task.ready());

  EXPECT_TRUE(task.resume());
  EXPECT_LE(steady_clock::now(), before + (duration * 2));
  EXPECT_EQ(counter, 2);
  EXPECT_FALSE(task.done());
  EXPECT_FALSE(task.ready());

  std::this_thread::sleep_for(duration);
  EXPECT_GE(steady_clock::now(), before + (duration * 2));
  EXPECT_EQ(counter, 2);
  EXPECT_FALSE(task.done());
  EXPECT_TRUE(task.ready());

  EXPECT_FALSE(task.resume());
  EXPECT_GE(steady_clock::now(), before + (duration * 2));
  EXPECT_EQ(counter, 3);
  EXPECT_TRUE(task.done());
}

}
}
