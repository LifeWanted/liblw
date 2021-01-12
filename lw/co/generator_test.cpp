#include "lw/co/generator.h"

#include "gtest/gtest.h"
#include "lw/co/future.h"
#include "lw/co/scheduler.h"
#include "lw/co/task.h"
#include "lw/co/testing/destroy_scheduler.h"

namespace lw::co {
namespace {

Generator<int> yield_some() {
  co_yield 1;
  co_yield 2;
}

AsyncGenerator<int> yield_async() {
  co_await next_tick();
  co_yield 1;
  co_await next_tick();
  co_yield make_resolved_future(2);
}

TEST(Generator, BasicGeneration) {
  auto gen = yield_some();
  int expected = 1;
  while (gen.next()) {
    EXPECT_EQ(gen.value(), expected);
    ++expected;
  }
  EXPECT_EQ(expected, 3);
}

TEST(Generator, RangeForGeneration) {
  int expected = 0;
  for (int i : yield_some()) {
    EXPECT_EQ(i, ++expected);
  }
  EXPECT_EQ(expected, 2);
}

TEST(AsyncGenerator, AsyncGeneration) {
  Scheduler::this_thread().schedule([]() -> Task<void> {
    auto gen = yield_async();
    int expected = 1;
    while (co_await gen.next()) {
      EXPECT_EQ(gen.value(), expected);
      ++expected;
    }
    EXPECT_EQ(expected, 3);
  });
  Scheduler::this_thread().run();
  testing::destroy_all_schedulers();
}

}
}
