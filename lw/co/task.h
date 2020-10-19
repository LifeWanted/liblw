#pragma once

#include <coroutine>
#include <exception>
#include <functional>
#include <memory>
#include <type_traits>

#include "lw/err/canonical.h"

namespace lw::co {

template <typename T>
class Task;

template <typename T>
class Promise {
public:
  Promise() = default;
  ~Promise() = default;

  Promise(Promise&&) = delete;
  Promise(const Promise&) = delete;
  Promise& operator=(Promise&&) = delete;
  Promise& operator=(const Promise&) = delete;

  auto initial_suspend() const {
    return std::suspend_always{};
  }

  auto final_suspend() const {
    return std::suspend_always{};
  }

  Task<T> get_return_object();

  auto return_void() const {
    throw Internal() << "Non-void coroutine returned void!";
  }

  template <typename U>
  auto return_value(U&& value) {
    _value = std::make_unique<T>(std::forward<U>(value));
    _valid = true;
    return std::suspend_never{};
  }

  void unhandled_exception() {
    _error = std::current_exception();
    _valid = true;
  }

private:
  bool _valid = false;
  std::unique_ptr<T> _value;
  std::exception_ptr _error;
  friend class Task<T>;
};

template <>
class Promise<void> {
public:
  Promise() = default;
  ~Promise() = default;

  Promise(Promise&&) = delete;
  Promise(const Promise&) = delete;
  Promise& operator=(Promise&&) = delete;
  Promise& operator=(const Promise&) = delete;

  auto initial_suspend() const {
    return std::suspend_always{};
  }

  auto final_suspend() const {
    return std::suspend_always{};
  }

  Task<void> get_return_object();

  auto return_void() {
    _valid = true;
    return std::suspend_never{};
  }

  template <typename U>
  auto return_value(U&& value) const {
    throw Internal() << "Void coroutine returning a value!";
  }

  void unhandled_exception() {
    _error = std::current_exception();
    _valid = true;
  }

private:
  bool _valid = false;
  std::exception_ptr _error;
  friend class Task<void>;
};

// -------------------------------------------------------------------------- //

template <typename T>
class Task {
public:
  using promise_type = Promise<T>;
  using handle_type = std::coroutine_handle<promise_type>;

  explicit Task(handle_type handle): _handle{std::move(handle)} {}
  ~Task() {
    if (_handle.promise()._error) {
      std::rethrow_exception(_handle.promise()._error);
    }
  }

  bool done() {
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
  using promise_type = Promise<void>;
  using handle_type = std::coroutine_handle<promise_type>;

  explicit Task(handle_type handle): _handle{std::move(handle)} {}
  ~Task() {
    if (_handle.promise()._error) {
      std::rethrow_exception(_handle.promise()._error);
    }
  }

  bool done() {
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

// -------------------------------------------------------------------------- //

Task<void> Promise<void>::get_return_object() {
  return Task<void>{Task<void>::handle_type::from_promise(*this)};
}

template <typename T>
Task<T> Promise<T>::get_return_object() {
  return Task<T>{Task<T>::handle_type::from_promise(*this)};
}

}
