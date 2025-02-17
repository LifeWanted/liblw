#pragma once

#include <coroutine>
#include <exception>
#include <memory>
#include <utility>

#include "lw/co/future.h"

namespace lw::co {

template <typename T>
class GeneratorIterator;

template <typename T>
class Generator {
public:
  class promise_type;
  using handle_type = std::coroutine_handle<promise_type>;

  explicit Generator(handle_type handle) : _handle{std::move(handle)} {}
  ~Generator() = default;

  Generator(Generator&&) = default;
  Generator& operator=(Generator&&) = default;

  Generator(const Generator&) = delete;
  Generator& operator=(const Generator&) = delete;

  bool next() {
    _handle.resume();
    return !_handle.done();
  }

  const T& value() const { return *_handle.promise()._current_value; }

  T& value() { return *_handle.promise()._current_value; }

  GeneratorIterator<T> begin();
  GeneratorIterator<T> end();

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

  auto initial_suspend() const noexcept { return std::suspend_always{}; }
  auto final_suspend() const noexcept { return std::suspend_always{}; }

  Generator get_return_object() {
    return Generator{handle_type::from_promise(*this)};
  }

  auto return_void() const { return std::suspend_never{}; }

  template <typename U>
  auto yield_value(U&& value) {
    _current_value = std::make_unique<T>(std::forward<U>(value));
    return std::suspend_always{};
  }

  void unhandled_exception() { std::terminate(); }

private:
  std::unique_ptr<T> _current_value;
  friend class Generator<T>;
};

template <typename T>
class GeneratorIterator {
public:
  GeneratorIterator(Generator<T>* generator) : _generator{generator} {}

  GeneratorIterator& operator++() {
    if (_generator && !_generator->next()) _generator = nullptr;
    return *this;
  }

  T& operator*() { return _generator->value(); }
  const T& operator*() const { return _generator->value(); }

  template <typename U>
  bool operator==(const GeneratorIterator<U>& other) const {
    return _generator == other._generator;
  }

private:
  Generator<T>* _generator;
};

template <typename T>
GeneratorIterator<T> Generator<T>::begin() {
  if (!_handle.promise()._current_value) next();
  return GeneratorIterator<T>{_handle.done() ? nullptr : this};
}

template <typename T>
GeneratorIterator<T> Generator<T>::end() {
  return GeneratorIterator<T>{nullptr};
}

// -------------------------------------------------------------------------- //

template <typename T>
class AsyncGenerator {
public:
  class promise_type;

  AsyncGenerator() : _state{std::make_shared<SharedState>()} {};
  ~AsyncGenerator() = default;

  AsyncGenerator(AsyncGenerator&&) = default;
  AsyncGenerator& operator=(AsyncGenerator&&) = default;

  AsyncGenerator(const AsyncGenerator&) = delete;
  AsyncGenerator& operator=(const AsyncGenerator&) = delete;

  Future<bool> next() {
    if (_state->done) return make_resolved_future(false);
    _state->next = Promise<bool>{};
    return _state->next.get_future();
  }

  const T& value() const { return *_state->current_value; }

  T& value() { return *_state->current_value; }

private:
  struct SharedState {
    bool done = false;
    Promise<bool> next;
    std::unique_ptr<T> current_value;
  };
  std::shared_ptr<SharedState> _state;
};

template <typename T>
class AsyncGenerator<T>::promise_type {
public:
  auto initial_suspend() const { return std::suspend_never{}; }

  auto final_suspend() const noexcept { return std::suspend_never{}; }

  AsyncGenerator get_return_object() {
    AsyncGenerator generator;
    _state = generator._state;
    return generator;
  }

  void return_void() {
    _state->done = true;
    _state->next.set_value(false);
  }

  template <typename U>
  auto yield_value(U&& value) {
    _state->current_value = std::make_unique<T>(std::forward<U>(value));
    _state->next.set_value(true);
    return std::suspend_always{};
  }

  template <typename U>
  Future<void> yield_value(Future<U>&& value) {
    _state->current_value = std::make_unique<T>(co_await value);
    _state->next.set_value(true);
  }

  void unhandled_exception() {
    _state->next.set_exception(std::current_exception());
  }

private:
  std::shared_ptr<AsyncGenerator<T>::SharedState> _state;
};

} // namespace lw::co
