#pragma once

#include <type_traits>

namespace lw::co {

class Scheduler;

template <typename T>
concept BooleanTestable = std::is_convertible_v<T, bool>;

template <typename T>
concept Awaitable = requires(T a) {
  { a.await_ready() } -> BooleanTestable;
  { a.await_suspend() };
  { a.await_resume() };
};

template <typename T>
concept Schedulable = std::is_same_v<T, ::lw::co::Scheduler&>;

template <typename T>
concept SchedulerHolder = requires(const T a) {
  { a.scheduler() } -> Schedulable;
};

}
