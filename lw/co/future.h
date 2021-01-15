#pragma once

#include <atomic>
#include <coroutine>
#include <exception>
#include <memory>

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
  std::atomic_bool state_set = false;
  std::atomic_bool future_suspended = false;
  std::exception_ptr exception = nullptr;
  Scheduler* scheduler = nullptr;
  std::coroutine_handle<> handle;
};

template <typename T>
struct SharedPromiseState: public SharedPromiseState<void> {
  std::unique_ptr<T> value;
  std::unique_ptr<Future<void>> future_return = nullptr;
};

}

class [[nodiscard]] FutureBase {
public:
  FutureBase(const FutureBase&) = delete;
  FutureBase& operator=(const FutureBase&) = delete;

  FutureBase(FutureBase&&) = default;
  FutureBase& operator=(FutureBase&&) = default;

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
    if (!_state->state_set) {
      throw FailedPrecondition()
        << "Cannot resume future before state is set on promise.";
    }
    if (_state->exception) std::rethrow_exception(_state->exception);
    return std::move(
      *std::static_pointer_cast<internal::SharedPromiseState<T>>(_state)->value
    );
  }

private:
  explicit Future(std::shared_ptr<internal::SharedPromiseState<T>> state):
    FutureBase{
      std::static_pointer_cast<internal::SharedPromiseState<void>>(state)
    }
  {}

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

  void set_value(const T& value) {
    _claim_state_set();
    _state->value = std::make_unique<T>(value);
    _schedule_task();
  }

  void set_value(T&& value) {
    _claim_state_set();
    _state->value = std::make_unique<T>(std::move(value));
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

  Future<void> final_suspend() const {
    if (_state->future_return) co_await *_state->future_return;
  }

  Future<T> get_return_object() {
    return get_future();
  }

  template <typename U>
  void return_value(U&& value) {
    set_value(std::forward<U>(value));
  }

  template <typename U>
  void return_value(Future<U>&& value) {
    // TODO(#10): This logic causes a segmentation fault. See the test
    // `PromiseInt::CoReturnFuture` for more details.
    if (value.await_ready()) {
      set_value(value.await_resume());
    }

    _state->future_return = std::make_unique<Future<void>>(
      _await_return(std::move(value))
    );
    _state->scheduler = _state->future_return->_state->scheduler;
    _state->handle = _state->future_return->_state->handle;
    _state->future_suspended = true;
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

  template <typename U>
  Future<void> _await_return(Future<U>&& value) {
    set_value(co_await value);
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

}
