#pragma once

#include <coroutine>
#include <exception>

#include "lw/co/coroutine_promise.h"
#include "lw/err/canonical.h"

namespace lw::co {

template <typename T>
class Task {
public:
  using promise_type = CoroutinePromise<Task<T>>;
  using handle_type = std::coroutine_handle<promise_type>;

  explicit Task(handle_type handle): _handle{std::move(handle)} {}
  ~Task() {
    if (_handle.promise()._error) {
      std::rethrow_exception(_handle.promise()._error);
    }
  }

  bool done() const {
    return _handle.done();
  }

  bool resume() {
    _handle.resume();
    return !_handle.done();
  }

  T get() {
    if (!_handle.promise()._valid) {
      throw FailedPrecondition()
        << "Task is not in a valid state before fetching the value. Check "
           "Task::done() before calling Task::get() and only call Task::get() "
           "once per Task instance.";
    }
    _handle.promise()._valid = false;
    if (_handle.promise()._error) {
      std::exception_ptr error{_handle.promise()._error};
      _handle.promise()._error = nullptr;
      std::rethrow_exception(error);
    }
    return std::move(*_handle.promise()._value);
  }

private:
  handle_type _handle;
};

template <>
class Task<void> {
public:
  using promise_type = CoroutinePromise<Task<void>>;
  using handle_type = std::coroutine_handle<promise_type>;

  explicit Task(handle_type handle): _handle{std::move(handle)} {}
  ~Task() {
    if (_handle.promise()._error) {
      std::rethrow_exception(_handle.promise()._error);
    }
  }

  bool done() const {
    return _handle.done();
  }

  bool resume() {
    _handle.resume();
    return !_handle.done();
  }

  void get() {
    if (!_handle.promise()._valid) {
      throw FailedPrecondition()
        << "Task is not in a valid state before fetching the value. Check "
           "Task::done() before calling Task::get() and only call Task::get() "
           "once per Task instance.";
    }
    _handle.promise()._valid = false;
    if (_handle.promise()._error) {
      std::exception_ptr error{_handle.promise()._error};
      _handle.promise()._error = nullptr;
      std::rethrow_exception(error);
    }
  }

private:
  handle_type _handle;
};

}
