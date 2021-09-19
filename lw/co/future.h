#pragma once

#include <atomic>
#include <concepts>
#include <coroutine>
#include <exception>
#include <memory>
#include <optional>
#include <tuple>
#include <type_traits>

#include "lw/co/scheduler.h"
#include "lw/err/canonical.h"

namespace lw::co {

template <typename T>
class Future;

template <typename T>
class Promise;

namespace internal {

template <typename T>
struct SharedPromiseState;

template <>
struct SharedPromiseState<void> {
  virtual ~SharedPromiseState() = default;

  std::atomic_bool state_set = false;
  std::atomic_bool future_suspended = false;
  std::exception_ptr exception = nullptr;
  Scheduler* scheduler = nullptr;
  std::coroutine_handle<> handle;
};

template <typename T>
struct SharedPromiseState: public SharedPromiseState<void> {
  std::optional<T> value;
  std::shared_ptr<SharedPromiseState<T>> chain_state = nullptr;
};

}

class [[nodiscard]] FutureBase {
public:
  FutureBase(const FutureBase&) = delete;
  FutureBase& operator=(const FutureBase&) = delete;

  FutureBase(FutureBase&&) = default;
  FutureBase& operator=(FutureBase&& other) = default;

  virtual ~FutureBase() = default;

  bool await_ready() const {
    return _state->state_set;
  }

  bool await_suspend(std::coroutine_handle<> handle) {
    _state->scheduler = &Scheduler::this_thread();
    _state->handle = handle;

    // TODO(alaina): Check this for thread safety. This statement isn't exactly
    // atomic, and I don't know if there is a race condition around setting the
    // state flag and actually setting the state and how that will interact with
    // coroutine logic the compiler generates around calling the await_*
    // methods.
    //
    // Possible test case involves a type with a mutex block in its copy
    // constructor to force a delay between setting the flag and setting the
    // value.
    //
    // Possible solution would be to combine state booleans into a single
    // bitmask variable. Then compare_and_exchange could be used in a loop to
    // check condition while setting attributes.
    return _state->future_suspended = !_state->state_set;
  }

protected:
  explicit FutureBase(
    std::shared_ptr<internal::SharedPromiseState<void>> state
  ):
    _state{state}
  {}

  std::shared_ptr<internal::SharedPromiseState<void>> _state;
};

template <typename T>
class [[nodiscard]] Future: public FutureBase {
public:
  using promise_type = Promise<T>;

  T await_resume() {
    auto state = _get_state();
    if (!state->state_set) {
      throw FailedPrecondition()
        << "Cannot resume future before state is set on promise.";
    }
    if (state->exception) std::rethrow_exception(state->exception);
    return std::move(*state->value);
  }

private:
  explicit Future(std::shared_ptr<internal::SharedPromiseState<T>> state):
    FutureBase{
      std::static_pointer_cast<internal::SharedPromiseState<void>>(state)
    }
  {}

  std::shared_ptr<internal::SharedPromiseState<T>> _get_state() {
    auto state = std::static_pointer_cast<internal::SharedPromiseState<T>>(
      _state
    );
    while (state->chain_state) state = state->chain_state;
    return state;
  }

  friend class Promise<T>;
};

template <>
class [[nodiscard]] Future<void>: public FutureBase {
public:
  using promise_type = Promise<void>;

  void await_resume() {
    if (!_state->state_set) {
      throw FailedPrecondition()
        << "Cannot resume future before state is set on promise.";
    }
    if (_state->exception) std::rethrow_exception(_state->exception);
  }

private:
  explicit Future(std::shared_ptr<internal::SharedPromiseState<void>> state):
    FutureBase{state}
  {}

  template <typename T>
  friend class Promise;
};

/**
 * A coroutine friendly drop in for `std::promise`.
 *
 * TODO(alaina): Check that the state transitions are thread safe!
 */
template <typename T>
class [[nodiscard]] Promise {
public:
  Promise():
    _state{std::make_shared<internal::SharedPromiseState<T>>()}
  {}

  Promise(const Promise&) = delete;
  Promise& operator=(const Promise&) = delete;
  Promise(Promise&& other):
    _future_obtained{other._future_obtained.load()},
    _state{std::move(other._state)}
  {}
  Promise& operator=(Promise&& other) {
    _future_obtained = other._future_obtained.load();
    _state = std::move(other._state);
    return *this;
  }

  /*** std::promise API ***/

  Future<T> get_future() {
    bool future_already_obtained = _future_obtained.exchange(true);
    if (future_already_obtained) {
      throw FailedPrecondition()
        << "Future already obtained previously.";
    }
    return Future<T>{_state};
  }

  template <typename U>
  void set_value(U&& value) {
    _do_set_value(std::forward<U>(value));
  }

  void set_exception(std::exception_ptr err) {
    auto state = _get_state();
    _claim_state_set(state);
    state->exception = err;
    _schedule_task(state);
  }

  /*** Coroutine promise hooks ***/

  auto initial_suspend() const noexcept { return std::suspend_never{}; }
  auto final_suspend() const noexcept { return std::suspend_never{}; }

  Future<T> get_return_object() {
    return get_future();
  }

  template <typename U>
  void return_value(U&& value) {
    set_value(std::forward<U>(value));
  }

  void return_value(Future<T>&& value) {
    auto val_state = std::static_pointer_cast<internal::SharedPromiseState<T>>(
      value._state
    );

    if (val_state->state_set) {
      if (val_state->exception) {
        set_exception(val_state->exception);
      } else {
        set_value(std::move(*val_state->value));
      }
    } else {
      val_state->chain_state = _state;
    }
  }

  void unhandled_exception() {
    set_exception(std::current_exception());
  }

private:
  using state_ptr = std::shared_ptr<internal::SharedPromiseState<T>>;

  template <typename U>
  void _do_set_value(U&& value) {
    auto state = _get_state();
    _claim_state_set(state);
    state->value = std::forward<U>(value);
    _schedule_task(state);
  }

  void _claim_state_set(state_ptr& state) {
    bool state_already_set = state->state_set.exchange(true);
    if (state_already_set) {
      throw FailedPrecondition()
        << "Promise state already set previously.";
    }
  }

  void _schedule_task(state_ptr& state) {
    if (state->future_suspended) state->scheduler->schedule(state->handle);
  }

  std::shared_ptr<internal::SharedPromiseState<T>> _get_state() {
    auto state = _state;
    while (state->chain_state) state = state->chain_state;
    return state;
  }

  std::atomic_bool _future_obtained = false;
  std::shared_ptr<internal::SharedPromiseState<T>> _state;
};

template <>
class [[nodiscard]] Promise<void> {
public:
  Promise():
    _state{std::make_shared<internal::SharedPromiseState<void>>()}
  {}

  Promise(const Promise&) = delete;
  Promise& operator=(const Promise&) = delete;
  Promise(Promise&& other):
    _future_obtained{other._future_obtained.load()},
    _state{std::move(other._state)}
  {}
  Promise& operator=(Promise&& other) {
    _future_obtained = other._future_obtained.load();
    _state = std::move(other._state);
    return *this;
  }

  /*** std::promise API ***/

  Future<void> get_future() {
    bool future_already_obtained = _future_obtained.exchange(true);
    if (future_already_obtained) {
      throw FailedPrecondition()
        << "Future already obtained previously.";
    }
    return Future<void>{_state};
  }

  void set_value() {
    _claim_state_set();
    _schedule_task();
  }

  void set_exception(std::exception_ptr err) {
    _claim_state_set();
    _state->exception = err;
    _schedule_task();
  }

  /*** Coroutine promise hooks ***/

  auto initial_suspend() const {
    return std::suspend_never{};
  }

  auto final_suspend() const noexcept {
    return std::suspend_never{};
  }

  Future<void> get_return_object() {
    return get_future();
  }

  void return_void() {
    set_value();
  }

  void unhandled_exception() {
    set_exception(std::current_exception());
  }
private:
  void _claim_state_set() {
    bool state_already_set = _state->state_set.exchange(true);
    if (state_already_set) {
      throw FailedPrecondition()
        << "Promise state already set previously.";
    }
  }

  void _schedule_task() {
    if (_state->future_suspended) _state->scheduler->schedule(_state->handle);
  }

  std::atomic_bool _future_obtained = false;
  std::shared_ptr<internal::SharedPromiseState<void>> _state;
};

// -------------------------------------------------------------------------- //

/**
 * Creates a future that is already resolved with the given value.
 */
template <typename T>
Future<T> make_resolved_future(T&& value) {
  Promise<T> promise;
  promise.set_value(std::forward<T>(value));
  return promise.get_future();
}

inline Future<void> make_resolved_future() {
  Promise<void> promise;
  promise.set_value();
  return promise.get_future();
}

/**
 * Create a future that is rejected with the given exception.
 */
template <typename T>
Future<T> make_resolved_future(std::exception_ptr err) {
  Promise<T> promise;
  promise.set_exception(err);
  return promise.get_future();
}

namespace internal {

template <typename T>
struct TupleAwaitResult {
  typedef T result_type;
  static co::Future<T> await(co::Future<T> future) { return future; }
};

template <>
struct TupleAwaitResult<void> {
  typedef std::nullptr_t result_type;
  static co::Future<std::nullptr_t> await(co::Future<void> future) {
    co_await future;
    co_return nullptr;
  }
};

}

/**
 * Await all the provided futures concurrently.
 *
 * Any `void` futures will resolve to a `nullptr` in the results tuple.
 *
 * @return
 *  A tuple containing all the resolved values from the futures in the order
 *  they are specified in the arguments.
 */
template <typename... Args>
co::Future<
  std::tuple<typename internal::TupleAwaitResult<Args>::result_type...>
> all(co::Future<Args>... futures) {
  co_return std::make_tuple(
    (co_await internal::TupleAwaitResult<Args>::await(std::move(futures)))...
  );
}

/**
 * Awaits all of the futures concurrently.
 *
 * This method requires all provided futures to be `void` resolving.
 */
template <std::same_as<co::Future<void>>... Futures>
co::Future<void> all_void(Futures&&... futures) {
  (co_await futures, ...);
  co_return; // Cover empty parameter pack case.
}

/**
 * Awaits all futures in a container, disregarding the results.
 *
 * All futures must be `void` resolving.
 */
template <typename Iterator>
co::Future<void> all_void(Iterator begin, Iterator end) {
  static_assert(
    std::is_same_v<std::decay_t<decltype(*begin)>, co::Future<void>>,
    "`co::all_void` must be called with only void futures."
  );
  for (auto itr = begin; itr != end; ++itr) co_await *itr;
}

}
