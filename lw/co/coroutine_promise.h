#pragma once

#include <coroutine>
#include <exception>
#include <memory>

#include "lw/err/canonical.h"

namespace lw::co {

template <typename T>
class CoroutinePromise;

template <template<typename> typename Ret, typename T>
class CoroutinePromise<Ret<T>> {
public:
  CoroutinePromise() = default;
  ~CoroutinePromise() = default;

  CoroutinePromise(CoroutinePromise&&) = delete;
  CoroutinePromise(const CoroutinePromise&) = delete;
  CoroutinePromise& operator=(CoroutinePromise&&) = delete;
  CoroutinePromise& operator=(const CoroutinePromise&) = delete;

  auto initial_suspend() const {
    return std::suspend_always{};
  }

  auto final_suspend() const {
    return std::suspend_always{};
  }

  Ret<T> get_return_object() {
    return Ret<T>{Ret<T>::handle_type::from_promise(*this)};
  }

  template <typename U>
  void return_value(U&& value) {
    _value = std::make_unique<T>(std::forward<U>(value));
    _valid = true;
  }

  void unhandled_exception() {
    _error = std::current_exception();
    _valid = true;
  }

private:
  bool _valid = false;
  std::unique_ptr<T> _value;
  std::exception_ptr _error;
  friend class Ret<T>;
};

template <template<typename> typename Ret >
class CoroutinePromise<Ret<void>> {
public:
  CoroutinePromise() = default;
  ~CoroutinePromise() = default;

  CoroutinePromise(CoroutinePromise&&) = delete;
  CoroutinePromise(const CoroutinePromise&) = delete;
  CoroutinePromise& operator=(CoroutinePromise&&) = delete;
  CoroutinePromise& operator=(const CoroutinePromise&) = delete;

  auto initial_suspend() const {
    return std::suspend_always{};
  }

  auto final_suspend() const {
    return std::suspend_always{};
  }

  Ret<void> get_return_object() {
      return Ret<void>{Ret<void>::handle_type::from_promise(*this)};
  }

  void return_void() {
    _valid = true;
  }

  void unhandled_exception() {
    _error = std::current_exception();
    _valid = true;
  }

private:
  bool _valid = false;
  std::exception_ptr _error;
  friend class Ret<void>;
};

}
