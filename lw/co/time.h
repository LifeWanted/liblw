#pragma once

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

}

template <typename TimePoint>
class SuspendUntil {
public:
  explicit SuspendUntil(const TimePoint& time_point): _time_point{time_point} {}
  SuspendUntil(SuspendUntil&&) = default;
  SuspendUntil& operator=(SuspendUntil&&) = default;
  ~SuspendUntil() = default;

  SuspendUntil(const SuspendUntil&) = delete;
  SuspendUntil& operator=(const SuspendUntil&) = delete;

  bool await_ready() const { return TimePoint::clock::now() >= _time_point; }

  template <typename T>
  void await_suspend(std::coroutine_handle<T>& handle) const {
    handle.promise().set_ready_check([this]() { return await_ready(); });
  }

  // void await_suspend(std::coroutine_handle<>) const {}
  void await_resume() const {
    std::this_thread::sleep_until(_time_point);
  }
private:
  TimePoint _time_point;
};

template <typename Rep, typename Period>
auto sleep_for(
  const std::chrono::duration<Rep, Period>& duration
) {
  return SuspendUntil<std::chrono::steady_clock::time_point>{
    std::chrono::steady_clock::now() + duration
  };
}

template <typename TimePoint>
auto sleep_until(const TimePoint& time_point) {
  return SuspendUntil<TimePoint>{time_point};
}

}