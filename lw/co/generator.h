#pragma once

#include <coroutine>
#include <cstdlib>
#include <utility>

namespace lw::co {

template <typename T>
class Generator {
public:
  class promise_type;
  using handle_type = std::coroutine_handle<promise_type>;

  explicit Generator(handle_type handle): _handle{std::move(handle)} {}
  ~Generator() = default;

  Generator(Generator&&) = default;
  Generator& operator=(Generator&&) = default;

  Generator(const Generator&) = delete;
  Generator& operator=(const Generator&) = delete;

  bool next() {
    _handle.resume();
    return !_handle.done();
  }

  const T& value() const {
    return _handle.promise()._current_value;
  }

  T value() {
    return _handle.promise()._current_value;
  }

private:
  handle_type _handle;
};

template <typename T>
class Generator<T>::promise_type {
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

  Generator get_return_object() {
    return Generator{handle_type::from_promise(*this)};
  }

  auto return_void() const {
    return std::suspend_never{};
  }

  template <typename U>
  auto yield_value(U&& value) {
    _current_value = std::forward<U>(value);
    return std::suspend_always{};
  }

  void unhandled_exception() {
    std::exit(1);
  }

private:
  T _current_value;
  friend class Generator<T>;
};

}
