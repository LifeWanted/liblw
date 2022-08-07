#include "lw/co/systems/grpc_event_system.h"

#include <chrono>
#include <limits>
#include <sys/timerfd.h>

#include "grpcpp/grpcpp.h"
#include "gtest/gtest.h"
#include "lw/co/events.h"
#include "lw/err/canonical.h"
#include "lw/flags/format.h"

namespace lw::co::internal {
namespace {

using std::chrono::high_resolution_clock;
using std::chrono::milliseconds;

TEST(GrpcEventSystem, EmptyTryWaitShouldNotBlock) {
  ::grpc::CompletionQueue queue;
  GrpcEventSystem events{queue};
  auto start = high_resolution_clock::now();
  EXPECT_EQ(events.try_wait(), 0);
  auto end = high_resolution_clock::now();

  EXPECT_LT(end - start, milliseconds(1))
    << "Elapsed time: " << cli::format(end - start);
}

TEST(GrpcEventSystem, TimerFd) {
  int timer = ::timerfd_create(CLOCK_REALTIME, TFD_NONBLOCK);
  ASSERT_GT(timer, 0) << "Timer file descripter.";
  ::itimerspec spec{
    .it_interval = {0},
    .it_value = {.tv_sec = 0, .tv_nsec = 15 * 1000000} // 15ms
  };
  auto start = high_resolution_clock::now();
  ASSERT_EQ(::timerfd_settime(timer, /*flags=*/0, &spec, nullptr), 0);

  ::grpc::CompletionQueue queue;
  GrpcEventSystem events{queue};
  bool called = false;
  events.add(
    timer,
    Event::READABLE | Event::ONE_SHOT,
    [&]() { called = true; }
  );

  EXPECT_FALSE(called);
  EXPECT_EQ(events.try_wait(), 0);
  ASSERT_LT(high_resolution_clock::now() - start, milliseconds(15));
  EXPECT_FALSE(called);
  EXPECT_EQ(events.wait_for(milliseconds(5)), 0);
  ASSERT_LT(high_resolution_clock::now() - start, milliseconds(15));
  EXPECT_FALSE(called);
  EXPECT_EQ(events.wait(), 1);
  EXPECT_GE(high_resolution_clock::now() - start, milliseconds(15));
  EXPECT_TRUE(called);
}

TEST(GrpcEventSystem, CheckTimeoutDurationBounds) {
  ::grpc::CompletionQueue queue;
  GrpcEventSystem events{queue};
  EXPECT_THROW(events.wait_for(milliseconds(-1)), InvalidArgument);
  EXPECT_THROW(
    events.wait_for(
      milliseconds(std::numeric_limits<milliseconds::rep>::max())
    ),
    InvalidArgument
  );
}

TEST(GrpcEventSystem, RejectAlreadyAddedFileDescriptors) {
  int timer = ::timerfd_create(CLOCK_REALTIME, TFD_NONBLOCK);
  ::grpc::CompletionQueue queue;
  GrpcEventSystem events{queue};
  events.add(timer, Event::READABLE, []() {});
  EXPECT_THROW(events.add(timer, Event::READABLE, []() {}), AlreadyExists);
}

TEST(GrpcEventSystem, RejectUnknownFileDescriptorsOnRemoval) {
  int fd = 123;
  ::grpc::CompletionQueue queue;
  GrpcEventSystem events{queue};
  EXPECT_THROW(events.remove(fd), InvalidArgument);
}

}
}
