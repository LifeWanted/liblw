#pragma once

#include <atomic>
#include <concepts>
#include <coroutine>
#include <exception>
#include <expected>
#include <memory>
#include <optional>
#include <tuple>

#include "lw/err/macros.h"

namespace lw::co {

template <typename T>
class promise;
template <typename T>
class future;

namespace future_internal {
template <typename T>
class promise_base;
}

template <typename T>
concept NotVoid = !std::is_void_v<T>;

template <NotVoid T>
future<T> make_resolved_future(T&& value);

future<void> make_resolved_future();

/**
 * Create a future that is rejected with the given exception.
 */
template <typename T>
future<T> make_resolved_future(std::exception_ptr err);

// MARK: Future

namespace future_internal {

template <typename T>
class future_base {
public:
  using promise_type = promise<T>;

  // No copy.
  future_base(const future_base&) = delete;
  future_base& operator=(const future_base&) = delete;

  // Moveable.
  future_base(future_base&&) = default;
  future_base& operator=(future_base&&) = default;

  ~future_base() = default;

  bool await_ready() const { return _state->result != std::nullopt; }

  bool await_suspend(std::coroutine_handle<> handle) {
    _state->handle = handle;
    return !_state->result;
  }

  T await_resume() {
    LW_CHECK_NE(_state->result, std::nullopt);
    if (!_state->result->has_value()) {
      std::rethrow_exception(_state->result->error());
    }
    if constexpr (!std::is_void_v<T>) return std::move(*_state->result).value();
  }

protected:
  struct state {
    std::optional<std::expected<T, std::exception_ptr>> result = std::nullopt;
    std::optional<std::coroutine_handle<>> handle = std::nullopt;
    std::shared_ptr<state> chain_state = nullptr;

    void schedule_resume() {
      if (handle) { handle->resume(); }
      if (chain_state) {
        chain_state->result.emplace(*std::move(result));
        chain_state->schedule_resume();
      }
    }
  };

  explicit future_base(std::shared_ptr<state> state) : _state{state} {}

  std::shared_ptr<state> _state;
};

} // namespace future_internal

template <typename T>
class [[nodiscard]] future final : public future_internal::future_base<T> {
public:
  using promise_type = promise<T>;

private:
  friend class promise<T>;
  friend class future_internal::promise_base<T>;
  friend future<T> make_resolved_future<>(T&&);
  template <typename U>
  friend future<U> make_resolved_future(std::exception_ptr);

  using future_base = future_internal::future_base<T>;
  using future_base::_state;
  using future_base::future_base;
  using future_base::state;
};

template <>
class [[nodiscard]] future<void> final :
    public future_internal::future_base<void> {
public:
  using promise_type = promise<void>;

private:
  friend class promise<void>;
  friend class future_internal::promise_base<void>;
  friend future<void> make_resolved_future();
  template <typename T>
  friend future<T> make_resolved_future(std::exception_ptr);

  using future_base = future_internal::future_base<void>;
  using future_base::_state;
  using future_base::future_base;
  using future_base::state;
};

// -------------------------------------------------------------------------- //
// MARK: Promise

namespace future_internal {

template <typename T>
class promise_base {
public:
  /*** std::promise API ***/

  future<T> get_future() { return future<T>{_state}; }

  void set_exception(std::exception_ptr err) {
    _state->result.emplace(std::unexpected(std::move(err)));
    _state->schedule_resume();
  }

  /*** Coroutine interface ***/

  void unhandled_exception() { set_exception(std::current_exception()); }

  auto initial_suspend() const noexcept { return std::suspend_never{}; }
  auto final_suspend() const noexcept { return std::suspend_never{}; }

  future<T> get_return_object() { return get_future(); }

protected:
  std::shared_ptr<typename future<T>::state> _state =
      std::make_shared<typename future<T>::state>();
};

} // namespace future_internal

template <typename T>
class promise final : public future_internal::promise_base<T> {
  using promise_base = future_internal::promise_base<T>;

public:
  /*** std::promise API ***/

  template <std::convertible_to<T> U>
  void set_value(U&& value) {
    _state->result.emplace(std::forward<U>(value));
    _state->schedule_resume();
  }

  void set_value(future<T> future_value) {
    if (future_value._state->result.has_value()) {
      _set_value(*std::move(future_value._state));
    } else {
      LW_CHECK_EQ(future_value._state->chain_state, nullptr);
      future_value._state->chain_state = _state;
    }
  }

  using promise_base::set_exception;

  /*** Coroutine interface ***/

  template <typename U>
  void return_value(U&& value) {
    set_value(std::forward<U>(value));
  }

  using promise_base::unhandled_exception;

private:
  using promise_base::_state;

  void _set_value(future<T>::state& resolved_state) {
    if (resolved_state.result.value().has_value()) {
      set_value(std::move(resolved_state.result).value().value());
    } else {
      set_exception(std::move(resolved_state.result).value().error());
    }
  }
};

template <>
class promise<void> final : public future_internal::promise_base<void> {
  using promise_base = future_internal::promise_base<void>;

public:
  /*** std::promise API ***/

  void set_value() {
    _state->result.emplace();
    _state->schedule_resume();
  }

  void set_value(future<void> f) {
    if (f._state->result.has_value()) {
      _set_value(*f._state);
    } else {
      LW_CHECK_EQ(f._state->chain_state, nullptr);
      f._state->chain_state = _state;
    }
  }

  using promise_base::set_exception;

  /*** Coroutine interface ***/

  void return_void() { set_value(); }

  using promise_base::unhandled_exception;

private:
  using promise_base::_state;

  void _set_value(future<void>::state& resolved_state) {
    if (resolved_state.result.value().has_value()) {
      set_value();
    } else {
      set_exception(std::move(resolved_state.result).value().error());
    }
  }
};

// -------------------------------------------------------------------------- //
// MARK: Utilities

/**
 * Creates a future that is already resolved with the given value.
 */
template <NotVoid T>
future<T> make_resolved_future(T&& value) {
  future<T> f{std::make_shared<typename future<T>::state>()};
  f._state->result.emplace(std::forward<T>(value));
  return f;
}

future<void> make_resolved_future() {
  future<void> f{std::make_shared<future<void>::state>()};
  f._state->result.emplace();
  return f;
}

/**
 * Create a future that is rejected with the given exception.
 */
template <typename T>
future<T> make_resolved_future(std::exception_ptr err) {
  future<T> f{std::make_shared<typename future<T>::state>()};
  f._state->result.emplace(std::unexpected(std::move(err)));
  return f;
}

/**
 * Await all the provided futures concurrently.
 *
 * @return
 *  A tuple containing all the resolved values from the futures in the order
 *  they are specified in the arguments.
 */
template <typename... Args>
future<std::tuple<Args...>> all(future<Args>... futures) {
  // TODO(#14): Implement this method for `co::Future<void>`.
  co_return std::make_tuple((co_await futures)...);
}

} // namespace lw::co
