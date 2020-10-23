#pragma once

#include <coroutine>
#include <type_traits>

namespace lw::co {

class Scheduler;

template <typename T>
concept BooleanTestable = std::is_convertible_v<T, bool>;

template <typename T>
concept Awaitable = requires(T a) {
  { a.await_ready() } -> BooleanTestable;
  { a.await_suspend(std::declval<std::coroutine_handle<>>()) };
  { a.await_resume() };
};

/**
 * A CallbackCoroutine is a functor that returns either an `lw::co::Task<void>`
 * or a `std::coroutine_handle<>`.
 *
 * TODO(alaina): Add restrictions on return type of the functor.
 */
template <typename T>
concept CallableCoroutine = requires(T a) {
  { a() };
};

}
