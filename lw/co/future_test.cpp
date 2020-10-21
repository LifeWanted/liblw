#include "lw/co/future.h"

#include <exception>

#include "gtest/gtest.h"
#include "lw/co/scheduler.h"
#include "lw/co/task.h"
#include "lw/co/testing/destroy_scheduler.h"
#include "lw/err/canonical.h"

namespace lw::co {
namespace {

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
  Scheduler::this_thread().schedule(([]() -> Task<void> {
    Promise<int> p;
    Future<int> f = p.get_future();
    EXPECT_FALSE(f.await_ready());
    EXPECT_TRUE(f.await_suspend({}));
    co_return;
  })());
  Scheduler::this_thread().run();
  testing::destroy_all_schedulers();
}

TEST(PromiseInt, SignalsNoSuspensionWhenReady) {
  Scheduler::this_thread().schedule(([]() -> Task<void> {
    Promise<int> p;
    Future<int> f = p.get_future();
    p.set_value(42);
    EXPECT_TRUE(f.await_ready());
    EXPECT_FALSE(f.await_suspend({}));
    co_return;
  })());
  Scheduler::this_thread().run();
  testing::destroy_all_schedulers();
}

TEST(PromiseInt, FutureIsAwaitable) {
  Promise<int> p;
  int result = 0;
  Scheduler::this_thread().schedule(([&]() -> Task<void> {
    EXPECT_EQ(result, 0);
    result = 2;
    result = co_await p.get_future();
    EXPECT_EQ(result, 42);
  })());
  Scheduler::this_thread().schedule(([&]() -> Task<void> {
    co_await Scheduler::this_thread().next_tick();
    EXPECT_EQ(result, 2);
    p.set_value(42);
  })());
  Scheduler::this_thread().run();
  EXPECT_EQ(result, 42);
  testing::destroy_all_schedulers();
}

TEST(PromiseInt, ExceptionsAreThrown) {
  Promise<int> p;
  int result = 0;
  Scheduler::this_thread().schedule(([&]() -> Task<void> {
    EXPECT_EQ(result, 0);
    result = 2;
    EXPECT_THROW(result = co_await p.get_future(), InvalidArgument);
    EXPECT_EQ(result, 2);
    result = 3;
  })());
  Scheduler::this_thread().schedule(([&]() -> Task<void> {
    co_await Scheduler::this_thread().next_tick();
    EXPECT_EQ(result, 2);
    p.set_exception(std::make_exception_ptr(InvalidArgument() << "error!"));
  })());
  Scheduler::this_thread().run();
  EXPECT_EQ(result, 3);
  testing::destroy_all_schedulers();
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
