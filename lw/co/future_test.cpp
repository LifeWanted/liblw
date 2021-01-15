#include "lw/co/future.h"

#include <exception>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lw/co/scheduler.h"
#include "lw/co/task.h"
#include "lw/co/testing/destroy_scheduler.h"
#include "lw/err/canonical.h"

namespace lw::co {
namespace {

using ::lw::co::testing::destroy_all_schedulers;
using ::testing::HasSubstr;

TEST(PromiseInt, ValueComesThroughFuture) {
  Promise<int> p;
  Future<int> f = p.get_future();
  p.set_value(42);
  EXPECT_TRUE(f.await_ready());
  EXPECT_EQ(f.await_resume(), 42);
}

TEST(PromiseInt, ExceptionComesThroughFuture) {
  Promise<int> p;
  Future<int> f = p.get_future();
  p.set_exception(std::make_exception_ptr(InvalidArgument() << "foobar"));
  EXPECT_TRUE(f.await_ready());
  EXPECT_THROW(f.await_resume(), InvalidArgument);
}

TEST(PromiseInt, DetectsReadiness) {
  Promise<int> p;
  Future<int> f = p.get_future();
  EXPECT_FALSE(f.await_ready());
  p.set_value(123);
  EXPECT_TRUE(f.await_ready());
}

TEST(PromiseInt, SignalsForSuspensionWhenNotReady) {
  Scheduler::this_thread().schedule([]() -> Task<void> {
    Promise<int> p;
    Future<int> f = p.get_future();
    EXPECT_FALSE(f.await_ready());
    EXPECT_TRUE(f.await_suspend({}));
    co_return;
  });
  Scheduler::this_thread().run();
  destroy_all_schedulers();
}

TEST(PromiseInt, SignalsNoSuspensionWhenReady) {
  Scheduler::this_thread().schedule([]() -> Task<void> {
    Promise<int> p;
    Future<int> f = p.get_future();
    p.set_value(42);
    EXPECT_TRUE(f.await_ready());
    EXPECT_FALSE(f.await_suspend({}));
    co_return;
  });
  Scheduler::this_thread().run();
  destroy_all_schedulers();
}

TEST(PromiseInt, FutureIsAwaitable) {
  Promise<int> p;
  int result = 0;
  Scheduler::this_thread().schedule([&]() -> Task<void> {
    EXPECT_EQ(result, 0);
    result = 2;
    result = co_await p.get_future();
    EXPECT_EQ(result, 42);
  });
  Scheduler::this_thread().schedule([&]() -> Task<void> {
    co_await next_tick();
    EXPECT_EQ(result, 2);
    p.set_value(42);
  });
  Scheduler::this_thread().run();
  EXPECT_EQ(result, 42);
  destroy_all_schedulers();
}

TEST(PromiseInt, ExceptionsAreThrown) {
  Promise<int> p;
  int result = 0;
  Scheduler::this_thread().schedule([&]() -> Task<void> {
    EXPECT_EQ(result, 0);
    result = 2;
    EXPECT_THROW(result = co_await p.get_future(), InvalidArgument);
    EXPECT_EQ(result, 2);
    result = 3;
  });
  Scheduler::this_thread().schedule([&]() -> Task<void> {
    co_await next_tick();
    EXPECT_EQ(result, 2);
    p.set_exception(std::make_exception_ptr(InvalidArgument() << "error!"));
  });
  Scheduler::this_thread().run();
  EXPECT_EQ(result, 3);
  destroy_all_schedulers();
}

TEST(PromiseInt, FutureCoroutine) {
  auto coro = [](int i) -> Future<int> {
    co_return i * 2;
  };
  int result = 0;
  Scheduler::this_thread().schedule([&]() -> Task<void> {
    ++result;
    co_await next_tick();
    ++result;
    result = co_await coro(result);
    ++result;
    EXPECT_EQ(result, 5);
    result = co_await coro(result);
  });
  Scheduler::this_thread().run();
  EXPECT_EQ(result, 10);
  destroy_all_schedulers();
}

// TODO(#10): This test causes a segmentation fault when resuming the
// `next_tick_coro`'s future. For some reason, the future thinks it has an
// exception and attempts to rethrow it. This rethrow triggers a seg fault.
//
// TEST(PromiseInt, CoReturnFuture) {
//   auto next_tick_coro = [](int i) -> Future<int> {
//     co_await next_tick();
//     co_return i * 2;
//   };
//   auto coro = [&](int i) -> Future<int> {
//     co_return next_tick_coro(i);
//   };
//   int result = 0;
//   Scheduler::this_thread().schedule([&]() -> Task<void> {
//     ++result;
//     co_await next_tick();
//     ++result;
//     result = co_await coro(result);
//     ++result;
//     EXPECT_EQ(result, 5);
//     result = co_await coro(result);
//   });
//   Scheduler::this_thread().run();
//   EXPECT_EQ(result, 10);
//   destroy_all_schedulers();
// }

TEST(PromiseInt, ThrowingFutureCoroutine) {
  auto coro = []() -> Future<int> {
    co_await next_tick();
    throw InvalidArgument() << "On noes!";
  };
  int result = 0;
  Scheduler::this_thread().schedule([&]() -> Task<void> {
    ++result;
    try {
      co_await coro();
    } catch (const InvalidArgument& err) {
      EXPECT_THAT(err.what(), HasSubstr("On noes"));
      ++result;
    }
  });
  Scheduler::this_thread().run();
  EXPECT_EQ(result, 2);
  destroy_all_schedulers();
}

// -------------------------------------------------------------------------- //

TEST(PromiseVoid, ValueComesThroughFuture) {
  Promise<void> p;
  Future<void> f = p.get_future();
  p.set_value();
  EXPECT_TRUE(f.await_ready());
  EXPECT_NO_THROW(f.await_resume());
}

TEST(PromiseVoid, ExceptionComesThroughFuture) {
  Promise<void> p;
  Future<void> f = p.get_future();
  p.set_exception(std::make_exception_ptr(InvalidArgument() << "foobar"));
  EXPECT_TRUE(f.await_ready());
  EXPECT_THROW(f.await_resume(), InvalidArgument);
}

TEST(PromiseVoid, DetectsReadiness) {
  Promise<void> p;
  Future<void> f = p.get_future();
  EXPECT_FALSE(f.await_ready());
  p.set_value();
  EXPECT_TRUE(f.await_ready());
}

TEST(PromiseVoid, SignalsForSuspensionWhenNotReady) {
  Scheduler::this_thread().schedule([]() -> Task<void> {
    Promise<void> p;
    Future<void> f = p.get_future();
    EXPECT_FALSE(f.await_ready());
    EXPECT_TRUE(f.await_suspend({}));
    co_return;
  });
  Scheduler::this_thread().run();
  destroy_all_schedulers();
}

TEST(PromiseVoid, SignalsNoSuspensionWhenReady) {
  Scheduler::this_thread().schedule([]() -> Task<void> {
    Promise<void> p;
    Future<void> f = p.get_future();
    p.set_value();
    EXPECT_TRUE(f.await_ready());
    EXPECT_FALSE(f.await_suspend({}));
    co_return;
  });
  Scheduler::this_thread().run();
  destroy_all_schedulers();
}

TEST(PromiseVoid, FutureIsAwaitable) {
  Promise<void> p;
  int result = 1;
  Scheduler::this_thread().schedule([&]() -> Task<void> {
    EXPECT_EQ(result, 1);
    result += result;
    co_await p.get_future();
    EXPECT_EQ(result, 4);
    result += result;
  });
  Scheduler::this_thread().schedule([&]() -> Task<void> {
    co_await next_tick();
    EXPECT_EQ(result, 2);
    result += result;
    p.set_value();
  });
  Scheduler::this_thread().run();
  EXPECT_EQ(result, 8);
  destroy_all_schedulers();
}

TEST(PromiseVoid, ExceptionsAreThrown) {
  Promise<void> p;
  int result = 0;
  Scheduler::this_thread().schedule([&]() -> Task<void> {
    EXPECT_EQ(result, 0);
    result = 2;
    EXPECT_THROW(co_await p.get_future(), InvalidArgument);
    EXPECT_EQ(result, 2);
    result = 3;
  });
  Scheduler::this_thread().schedule([&]() -> Task<void> {
    co_await next_tick();
    EXPECT_EQ(result, 2);
    p.set_exception(std::make_exception_ptr(InvalidArgument() << "error!"));
  });
  Scheduler::this_thread().run();
  EXPECT_EQ(result, 3);
  destroy_all_schedulers();
}

TEST(PromiseVoid, FutureCoroutine) {
  auto coro = [](int& i) -> Future<void> {
    i *= 2;
    co_return;
  };
  int result = 0;
  Scheduler::this_thread().schedule([&]() -> Task<void> {
    ++result;
    co_await next_tick();
    ++result;
    co_await coro(result);
    ++result;
    EXPECT_EQ(result, 5);
    co_await coro(result);
  });
  Scheduler::this_thread().run();
  EXPECT_EQ(result, 10);
  destroy_all_schedulers();
}

TEST(PromiseVoid, ThrowingFutureCoroutine) {
  auto coro = []() -> Future<void> {
    co_await next_tick();
    throw InvalidArgument() << "On noes!";
  };
  int result = 0;
  Scheduler::this_thread().schedule([&]() -> Task<void> {
    ++result;
    try {
      co_await coro();
    } catch (const InvalidArgument& err) {
      EXPECT_THAT(err.what(), HasSubstr("On noes"));
      ++result;
    }
  });
  Scheduler::this_thread().run();
  EXPECT_EQ(result, 2);
  destroy_all_schedulers();
}

// -------------------------------------------------------------------------- //

TEST(MakeFuture, ResolveVoid) {
  Future<void> f = make_resolved_future();
  EXPECT_TRUE(f.await_ready());
  EXPECT_NO_THROW(f.await_resume());
}

TEST(MakeFuture, ResolveToAValue) {
  Future<int> f = make_resolved_future(123);
  EXPECT_TRUE(f.await_ready());
  EXPECT_EQ(f.await_resume(), 123);
}

TEST(MakeFuture, RejectAValue) {
  Future<int> f = make_resolved_future<int>(
    std::make_exception_ptr(InvalidArgument() << "foobar")
  );
  EXPECT_TRUE(f.await_ready());
  EXPECT_THROW(f.await_resume(), InvalidArgument);
}

}
}
