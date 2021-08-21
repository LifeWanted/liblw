#include "lw/co/task.h"

#include <coroutine>

#include "gtest/gtest.h"

namespace lw::co {
namespace {

Task test_task(int& counter) {
  ++counter;
  co_await std::suspend_always{};
  ++counter;
}

TEST(VoidTask, BasicUsage) {
  int counter = 0;
  auto task = test_task(counter);
  EXPECT_FALSE(task.done());
  EXPECT_EQ(counter, 0);
  EXPECT_TRUE(task.resume());
  EXPECT_FALSE(task.done());
  EXPECT_EQ(counter, 1);
  EXPECT_FALSE(task.resume());
  EXPECT_TRUE(task.done());
  EXPECT_EQ(counter, 2);
  EXPECT_NO_THROW(task.get());
}

TEST(VoidTask, Callback) {
  int counter = 0;
  int cb_called = 0;
  auto task = test_task(counter);
  task.then([&]() { ++cb_called; });

  EXPECT_EQ(cb_called, 0);
  while (!task.done()) task.resume();
  EXPECT_EQ(cb_called, 1);
}

}
}
