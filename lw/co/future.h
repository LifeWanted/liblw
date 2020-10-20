#pragma once

#include <atomic>
#include <exception>
#include <future>
#include <memory>

#include "lw/co/scheduler.h"
#include "lw/err/canonical.h"

namespace lw::co {
namespace internal {

template <typename T>
struct SharedPromiseState;

template <>
struct SharedPromiseState<void> {
  std::atomic_bool state_set = false;
  std::atomic_bool future_suspended = false;
  std::exception_ptr exception;
  Scheduler* scheduler = nullptr;
  TaskRef task;
};

template <typename T>
struct SharedPromiseState: public SharedPromiseState<void> {
  std::unique_ptr<T> value;
};

}

template <typename T>
class Promise;

template <typename T>
class [[nodiscard]] Future {
public:
  Future(const Future&) = delete;
  Future& operator=(const Future&) = delete;

  Future(Future&&) = default;
  Future& operator=(Future&&) = default;

  bool await_ready() const { return _state->state_set; }

  bool await_suspend(std::coroutine_handle<>) {
    _state->scheduler = &Scheduler::this_thread();
    _state->task = _state->scheduler->current_task();

    // TODO(alaina): Check this for thread safety. This statement, and the
    // equivalent one in Future<void> below, isn't exactly atomic, and I don't
    // know if there is a race condition around setting the state flag and
    // actually setting the state and how that will interact with coroutine
    // logic the compiler generates around calling the await_* methods.
    //
    // Possible test case involves a type with a mutex block in its copy
    // constructor to force a delay between setting the flag and setting the
    // value.
    return _state->future_suspended = !_state->state_set;
  }

  T await_resume() {
    if (!_state->state_set) {
      throw FailedPrecondition()
        << "Cannot resume future before state is set on promise.";
    }
    if (_state->exception) std::rethrow_exception(_state->exception);
    return std::move(*_state->value);
  }

private:
  explicit Future(std::shared_ptr<internal::SharedPromiseState<T>> state):
    _state{state}
  {}

  std::shared_ptr<internal::SharedPromiseState<T>> _state;

  friend class Promise<T>;
};

template <>
class [[nodiscard]] Future<void> {
public:
  bool await_ready() const { return _state->state_set; }

  bool await_suspend(std::coroutine_handle<>) {
    _state->scheduler = &Scheduler::this_thread();
    _state->task = _state->scheduler->current_task();
    return _state->future_suspended = !_state->state_set;
  }

  void await_resume() {
    if (!_state->state_set) {
      throw FailedPrecondition()
        << "Cannot resume future before state is set on promise.";
    }
    if (_state->exception) std::rethrow_exception(_state->exception);
  }

private:
  explicit Future(std::shared_ptr<internal::SharedPromiseState<void>> state):
    _state{state}
  {}

  std::shared_ptr<internal::SharedPromiseState<void>> _state;

  friend class Promise<void>;
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
  Promise(Promise&&);
  Promise& operator=(Promise&&);

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

  void set_value(T& value) {
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

private:
  void _claim_state_set() {
    bool state_already_set = _state->state_set.exchange(true);
    if (state_already_set) {
      throw FailedPrecondition()
        << "Promise state already set previously.";
    }
  }

  void _schedule_task() {
    if (_state->future_suspended) _state->scheduler->schedule(_state->task);
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
  Promise(Promise&&);
  Promise& operator=(Promise&&);

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

private:
  void _claim_state_set() {
    bool state_already_set = _state->state_set.exchange(true);
    if (state_already_set) {
      throw FailedPrecondition()
        << "Promise state already set previously.";
    }
  }

  void _schedule_task() {
    if (_state->future_suspended) _state->scheduler->schedule(_state->task);
  }

  std::atomic_bool _future_obtained = false;
  std::shared_ptr<internal::SharedPromiseState<void>> _state;
};

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
