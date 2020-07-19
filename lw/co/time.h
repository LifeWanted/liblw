#pragma once

#ifndef __cpp_impl_coroutine
#define __cpp_impl_coroutine 1
#endif

#include <chrono>
#include <coroutine>
#include <thread>

#include "lw/co/scheduler.h"

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

Handle create_timerfd(ClockType type, std::chrono::nanoseconds expiration);

template <typename T>
Scheduler& pick_scheduler(const std::coroutine_handle<T>&);

template <>
inline Scheduler& pick_scheduler(const std::coroutine_handle<void>&) {
  return Scheduler::this_thread();
}

template <SchedulerHolder Promise>
inline Scheduler& pick_scheduler(const std::coroutine_handle<Promise>& handle) {
  return handle.promise().scheduler();
}

}

template <typename Clock, typename Duration>
class SuspendUntil {
public:
  typedef std::chrono::time_point<Clock, Duration> TimePoint;

  explicit SuspendUntil(const TimePoint& time_point):
    _time_point{time_point},
    _timer{nullptr}
  {}

  SuspendUntil(SuspendUntil&&) = default;
  SuspendUntil& operator=(SuspendUntil&&) = default;
  ~SuspendUntil() = default;

  SuspendUntil(const SuspendUntil&) = delete;
  SuspendUntil& operator=(const SuspendUntil&) = delete;

  bool await_ready() const { return TimePoint::clock::now() >= _time_point; }

  template <typename T>
  void await_suspend(std::coroutine_handle<T>& handle) const {
    if (!_timer) {
      _timer = internal::create_timerfd(
        internal::get_clock_type<Clock>(),
        _time_point.time_since_epoch()
      );
    }
    internal::pick_scheduler(handle).schedule(_timer, Event::READABLE, *this);
  }

  void await_resume() const {
    std::this_thread::sleep_until(_time_point);
  }

private:
  TimePoint _time_point;
  Handle _timer;
};

template <typename Rep, typename Period>
auto sleep_for(
  const std::chrono::duration<Rep, Period>& duration
) {
  return SuspendUntil<std::chrono::steady_clock::time_point>{
    std::chrono::steady_clock::now() + duration
  };
}

template <typename Clock, typename Duration>
auto sleep_until(const std::chrono::time_point<Clock, Duration>& time_point) {
  return SuspendUntil<Clock, Duration>{time_point};
}

}
