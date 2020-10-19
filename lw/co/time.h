#pragma once

#include <chrono>
#include <coroutine>
#include <thread>

#include "lw/co/scheduler.h"
#include "lw/co/task.h"

namespace lw::co {
namespace internal {

enum ClockType {
  STEADY,
  SYSTEM
};

template <typename Clock>
constexpr ClockType get_clock_type();

template <>
constexpr ClockType get_clock_type<std::chrono::steady_clock>() {
  return ClockType::STEADY;
}

template <>
constexpr ClockType get_clock_type<std::chrono::system_clock>() {
  return ClockType::SYSTEM;
}

Handle create_timerfd(ClockType type, std::chrono::nanoseconds duration);
void close_timerfd(Handle timer);

}

template <typename Clock, typename Duration>
class SuspendFor {
public:
  typedef std::chrono::time_point<Clock, Duration> TimePoint;

  explicit SuspendFor(const Duration& duration):
    _duration{duration},
    _timer{0}
  {}

  SuspendFor(SuspendFor&&) = default;
  SuspendFor& operator=(SuspendFor&&) = default;
  ~SuspendFor() = default;

  SuspendFor(const SuspendFor&) = default;
  SuspendFor& operator=(const SuspendFor&) = default;

  bool await_ready() const { return false; }

  void await_suspend(std::coroutine_handle<>) {
    if (!_timer) {
      _timer = internal::create_timerfd(
        internal::get_clock_type<Clock>(),
        _duration
      );
    }
    Scheduler::this_thread()
      .schedule(_timer, Event::READABLE | Event::ONE_SHOT);
  }

  void await_resume() {
    internal::close_timerfd(_timer);
  }

private:
  Duration _duration;
  Handle _timer;
};

template <typename Rep, typename Period>
auto sleep_for(
  const std::chrono::duration<Rep, Period>& duration
) {
  return SuspendFor<
    std::chrono::steady_clock,
    std::chrono::duration<Rep, Period>
  >{
    duration
  };
}

template <typename Clock, typename Duration>
auto sleep_until(const std::chrono::time_point<Clock, Duration>& time_point) {
  return SuspendFor<Clock, Duration>{time_point - Clock::now()};
}

}
