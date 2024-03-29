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
  Scheduler::this_thread().schedule([]() -> Task {
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
  Scheduler::this_thread().schedule([]() -> Task {
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
  Scheduler::this_thread().schedule([&]() -> Task {
    EXPECT_EQ(result, 0);
    result = 2;
    result = co_await p.get_future();
    EXPECT_EQ(result, 42);
  });
  Scheduler::this_thread().schedule([&]() -> Task {
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
  Scheduler::this_thread().schedule([&]() -> Task {
    EXPECT_EQ(result, 0);
    result = 2;
    EXPECT_THROW(result = co_await p.get_future(), InvalidArgument);
    EXPECT_EQ(result, 2);
    result = 3;
  });
  Scheduler::this_thread().schedule([&]() -> Task {
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
  Scheduler::this_thread().schedule([&]() -> Task {
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

TEST(PromiseInt, CoReturnSetFuture) {
  auto next_tick_coro = [](int i) -> Future<int> {
    co_return i * 2;
  };
  auto coro = [&](int i) -> Future<int> {
    co_return next_tick_coro(i);
  };
  int result = 0;
  Scheduler::this_thread().schedule([&]() -> Task {
    result += 2;
    result = co_await coro(result);
    ++result;
    EXPECT_EQ(result, 5);
  });
  Scheduler::this_thread().run();
  EXPECT_EQ(result, 5);
  destroy_all_schedulers();
}

TEST(PromiseInt, CoReturnSuspendedFuture) {
  auto next_tick_coro = [](int i) -> Future<int> {
    co_await next_tick();
    co_return i * 2;
  };
  auto coro = [&](int i) -> Future<int> {
    co_return next_tick_coro(i);
  };
  int result = 0;
  Scheduler::this_thread().schedule([&]() -> Task {
    result += 2;
    result = co_await coro(result);
    ++result;
    EXPECT_EQ(result, 5);
  });
  Scheduler::this_thread().run();
  EXPECT_EQ(result, 5);
  destroy_all_schedulers();
}

TEST(PromiseInt, ThrowingFutureCoroutine) {
  auto coro = []() -> Future<int> {
    co_await next_tick();
    throw InvalidArgument() << "On noes!";
  };
  int result = 0;
  Scheduler::this_thread().schedule([&]() -> Task {
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

TEST(PromiseInt, CoReturnThrownFuture) {
  auto next_tick_coro = [](int i) -> Future<int> {
    throw InvalidArgument() << "On noes!";
  };
  auto coro = [&](int i) -> Future<int> {
    co_return next_tick_coro(i);
  };
  int result = 0;
  Scheduler::this_thread().schedule([&]() -> Task {
    ++result;
    try {
      result = co_await coro(result);
      result += 2;
    } catch (const InvalidArgument& err) {
      EXPECT_THAT(err.what(), HasSubstr("On noes"));
      ++result;
    }
    EXPECT_EQ(result, 2);
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
  Scheduler::this_thread().schedule([]() -> Task {
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
  Scheduler::this_thread().schedule([]() -> Task {
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
  Scheduler::this_thread().schedule([&]() -> Task {
    EXPECT_EQ(result, 1);
    result += result;
    co_await p.get_future();
    EXPECT_EQ(result, 4);
    result += result;
  });
  Scheduler::this_thread().schedule([&]() -> Task {
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
  Scheduler::this_thread().schedule([&]() -> Task {
    EXPECT_EQ(result, 0);
    result = 2;
    EXPECT_THROW(co_await p.get_future(), InvalidArgument);
    EXPECT_EQ(result, 2);
    result = 3;
  });
  Scheduler::this_thread().schedule([&]() -> Task {
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
  Scheduler::this_thread().schedule([&]() -> Task {
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
  Scheduler::this_thread().schedule([&]() -> Task {
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

TEST(AwaitAll, AllResolve) {
  Scheduler::this_thread().schedule([]() -> Task {
    std::tuple<int, float, double> res = co_await all(
      make_resolved_future<int>(1),
      make_resolved_future<float>(2.0f),
      make_resolved_future<double>(3.14)
    );
    EXPECT_EQ(std::get<int>(res), 1);
    EXPECT_EQ(std::get<float>(res), 2.0f);
    EXPECT_EQ(std::get<double>(res), 3.14);
  });
  Scheduler::this_thread().run();
  destroy_all_schedulers();
}

// TODO(#14): Implement support for Future<void> in co::all.
// TEST(AwaitAll, AllVoid) {
//   int counter = 0;
//   auto coro0 = [&]() -> Future<void> {
//     co_await next_tick();
//     EXPECT_EQ(++counter, 1);
//   };
//   auto coro1 = [&]() -> Future<void> {
//     co_await next_tick();
//     EXPECT_EQ(++counter, 2);
//   };
//   auto coro2 = [&]() -> Future<void> {
//     co_await next_tick();
//     EXPECT_EQ(++counter, 3);
//   };

//   Scheduler::this_thread().schedule([&]() -> Task {
//     co_await all(coro0(), coro1(), coro2());
//     EXPECT_EQ(counter, 3);
//   });
//   Scheduler::this_thread().run();
//   destroy_all_schedulers();
// }

TEST(AwaitAll, ResolveOutOfOrder) {
  int counter = 0;
  auto coro0 = [&]() -> Future<int> {
    EXPECT_EQ(counter, 0);
    co_await next_tick();
    EXPECT_EQ(counter, 1);
    co_await next_tick();
    EXPECT_EQ(counter, 2);
    co_return ++counter;
  };
  auto coro1 = [&]() -> Future<int> {
    EXPECT_EQ(counter, 0);
    co_await next_tick();
    EXPECT_EQ(counter, 1);
    co_return ++counter;
  };
  auto coro2 = [&]() -> Future<int> {
    EXPECT_EQ(counter, 0);
    co_return ++counter;
  };

  Scheduler::this_thread().schedule([&]() -> Task {
    auto res = co_await all(coro0(), coro1(), coro2());
    EXPECT_EQ(std::get<0>(res), 3);
    EXPECT_EQ(std::get<1>(res), 2);
    EXPECT_EQ(std::get<2>(res), 1);
  });
  Scheduler::this_thread().run();
  destroy_all_schedulers();
}

}
}
