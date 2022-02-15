#pragma once

#include <coroutine>
#include <exception>
#include <functional>
#include <memory>

#include "lw/err/canonical.h"

namespace lw::co {

class Task;

class TaskPromise {
public:
  TaskPromise(): _state{std::make_shared<State>()} {};
  ~TaskPromise() = default;

  TaskPromise(TaskPromise&&) = delete;
  TaskPromise(const TaskPromise&) = delete;
  TaskPromise& operator=(TaskPromise&&) = delete;
  TaskPromise& operator=(const TaskPromise&) = delete;

  Task get_return_object();

  auto initial_suspend() const noexcept { return std::suspend_always{}; }
  auto final_suspend() const noexcept { return std::suspend_always{}; }

  void unhandled_exception() { _state->error = std::current_exception(); }
  void return_void() {
    _state->finished = true;
    if (_state->callback) _state->callback();
  }

  bool valid() const { return _state->error || _state->finished; }
  std::exception_ptr error() const { return _state->error; }

private:
  friend class Task;
  struct State {
    std::exception_ptr error;
    std::function<void()> callback;
    bool finished = false;
  };
  std::shared_ptr<State> _state;
};

// -------------------------------------------------------------------------- //

class Task {
public:
  using promise_type = TaskPromise;
  using handle_type = std::coroutine_handle<promise_type>;

  explicit Task(handle_type handle, std::shared_ptr<TaskPromise::State> state):
    _handle{std::move(handle)},
    _state{std::move(state)}
  {}

  ~Task() {
    if (_handle.promise().error()) {
      std::rethrow_exception(_handle.promise().error());
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
    if (!_handle.promise().valid()) {
      throw FailedPrecondition()
        << "Task is not in a valid state before fetching the value. Check "
           "Task::done() before calling Task::get() and only call Task::get() "
           "once per Task instance.";
    }
    if (_handle.promise().error()) {
      std::rethrow_exception(_handle.promise().error());
    }
  }

  std::coroutine_handle<> handle() const {
    return static_cast<std::coroutine_handle<>>(_handle);
  }

  template <typename Func>
  void then(Func&& callback) {
    _state->callback = std::forward<Func>(callback);
  }

private:
  handle_type _handle;
  std::shared_ptr<TaskPromise::State> _state;
};

inline Task TaskPromise::get_return_object() {
  return Task{Task::handle_type::from_promise(*this), _state};
}

}
