#pragma once

#include <coroutine>
#include <exception>
#include <functional>
#include <type_traits>


#include "lw/err/canonical.h"

namespace lw::co {

template <typename T>
class Task {
public:
  class promise_type;
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
    return std::move(_handle.promise()._value);
  }

private:
  handle_type _handle;
};

template <typename T>
class Task<T>::promise_type {
public:
  promise_type() = default;
  ~promise_type() = default;

  promise_type(promise_type&&) = delete;
  promise_type(const promise_type&) = delete;
  promise_type& operator=(promise_type&&) = delete;
  promise_type& operator=(const promise_type&) = delete;

  auto initial_suspend() const {
    return std::suspend_always{};
  }

  auto final_suspend() const {
    return std::suspend_always{};
  }

  Task get_return_object() {
    return Task{handle_type::from_promise(*this)};
  }

  auto return_void() const {
    throw Internal() << "Non-void coroutine returned void!";
  }

  template <typename U>
  auto return_value(U&& value) {
    _value = std::forward<U>(value);
    _valid = true;
    return std::suspend_never{};
  }

  void unhandled_exception() {
    _error = std::current_exception();
    _valid = true;
  }

private:
  bool _valid = false;
  T _value;
  std::exception_ptr _error;
  friend class Task<T>;
};

// -------------------------------------------------------------------------- //

template <>
class Task<void> {
public:
  class promise_type {
  public:
    promise_type() = default;
    ~promise_type() = default;

    promise_type(promise_type&&) = delete;
    promise_type(const promise_type&) = delete;
    promise_type& operator=(promise_type&&) = delete;
    promise_type& operator=(const promise_type&) = delete;

    auto initial_suspend() const {
      return std::suspend_always{};
    }

    auto final_suspend() const {
      return std::suspend_always{};
    }

    Task get_return_object() {
      return Task{handle_type::from_promise(*this)};
    }

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

    template <typename Func>
    void set_ready_check(Func&& func) {
      _ready = std::forward<Func>(func);
    }

    bool ready() {
      return _ready ? _ready() : true;
    }
  private:
    bool _valid = false;
    std::exception_ptr _error;
    std::function<bool()> _ready;
    friend class Task<void>;
  };
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

  bool ready() {
    return _handle.promise().ready();
  }

  bool resume() {
    _handle.promise()._ready = nullptr;
    _handle.resume();
    return !_handle.done();
  }

  Task& wait() {
    while (!done()) resume();
    return *this;
  }

  // bool await_ready() {
  //   return ready();
  // }

  // handle_type& await_suspend(std::coroutine_handle<>) {
  //   return _handle;
  // }

  // void await_resume() {
  //   resume();
  // }

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

template <typename T>
class SubTaskAwaitable {
public:
  explicit SubTaskAwaitable(Task<T>& task) {

  }
};

}
