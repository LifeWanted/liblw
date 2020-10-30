#pragma once

#include <concepts>
#include <coroutine>

namespace lw::co {

class Scheduler;

template <typename T>
concept Awaitable = requires(T a) {
  { a.await_ready() } -> std::convertible_to<bool>;
  { a.await_suspend(std::declval<std::coroutine_handle<>>()) };
  { a.await_resume() };
};

template <typename T, typename Result>
concept ValueAwaitable = Awaitable<T> && requires(T a) {
  { a.await_resume() } -> std::convertible_to<Result>;
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
