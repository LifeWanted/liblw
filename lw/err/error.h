#pragma once

#include <experimental/source_location>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <type_traits>

#include "lw/io/stream/buffer.h"

namespace lw {

class Error;
template <typename ErrorType>
ErrorType wrap(
  const Error& sub_error,
  const std::experimental::source_location& loc =
    std::experimental::source_location::current()
);
template <typename ErrorType>
ErrorType wrap(
  Error&& sub_error,
  const std::experimental::source_location& loc =
    std::experimental::source_location::current()
);

class ErrorStack {
public:
  class Iterator {
  public:
    Iterator() = default;
    explicit Iterator(const ErrorStack& stack): _stack{&stack} {}

    bool operator==(const Iterator& rhs) const { return _stack == rhs._stack; }
    bool operator!=(const Iterator& rhs) const { return !(*this == rhs); }
    Iterator& operator++() {
      if (_stack) _stack = _stack->_previous.get();
      return *this;
    }
    Iterator operator++(int) {
      Iterator it{*this};
      ++(*this);
      return it;
    }

    const ErrorStack& operator*() const { return *_stack; }
    const ErrorStack* operator->() const { return _stack; }

  private:
    const ErrorStack* _stack = nullptr;
  };

  explicit ErrorStack(const std::experimental::source_location& loc);
  ErrorStack(const ErrorStack& other);
  ErrorStack& operator=(const ErrorStack& rhs);
  ErrorStack(ErrorStack&&) = default;
  ErrorStack& operator=(ErrorStack&&) = default;

  std::string_view message() const { return _buffer.string(); }
  const std::experimental::source_location& location() const noexcept {
    return _location;
  }

  Iterator begin() const { return Iterator{*this}; }
  Iterator end() const { return Iterator{}; }

private:
  friend class Error;
  friend class Iterator;

  std::experimental::source_location _location;
  io::stream::StringBuffer _buffer;
  std::unique_ptr<ErrorStack> _previous;
};

/**
 * @brief Base class for all exceptions thrown within liblw.
 *
 * It is a subclass of `std::runtime_error` with the addition of stream-like
 * error message formatting.
 */
class Error: public std::runtime_error {
public:
  Error(const Error& other);
  Error(Error&& other);
  Error& operator=(const Error& other);
  Error& operator=(Error&& other);

  const char* what() const noexcept override {
    return _stack._buffer.c_str();
  }
  const std::experimental::source_location& where() const noexcept {
    return _stack._location;
  }
  std::string_view what_string() const noexcept {
    return _stack._buffer.string();
  }

  const ErrorStack& stack() const { return _stack; }

protected:
  Error(std::string_view name, const std::experimental::source_location& loc);

private:
  template <typename T>
  friend Error& operator<<(Error&, T&&);
  template <typename T>
  friend Error&& operator<<(Error&&, T&&);
  template <typename ErrorType>
  friend ErrorType wrap(
    const Error&,
    const std::experimental::source_location&
  );
  template <typename ErrorType>
  friend ErrorType wrap(Error&&, const std::experimental::source_location&);

  void _set_previous(ErrorStack previous);

  ErrorStack _stack;
  std::ostream _stream;
};

// -------------------------------------------------------------------------- //

template <typename T>
Error& operator<<(Error& err, T&& val) {
  err._stream << std::forward<T>(val);
  return err;
}

template <typename T>
Error&& operator<<(Error&& err, T&& val) {
  err._stream << std::forward<T>(val);
  return std::move(err);
}

inline Error& operator<<(Error& err, const Error& rhs) {
  return err << rhs.what_string();
}

inline Error&& operator<<(Error&& err, const Error& rhs) {
  return std::move(err) << rhs.what_string();
}

template <
  typename ErrorType,
  typename T,
  std::enable_if<std::is_base_of<Error, ErrorType>::value>::type* = nullptr
>
ErrorType& operator<<(ErrorType& err, T&& val) {
  static_cast<Error&>(err) << std::forward<T>(val);
  return err;
}

template <
  typename ErrorType,
  typename T,
  std::enable_if<std::is_base_of<Error, ErrorType>::value>::type* = nullptr
>
ErrorType&& operator<<(ErrorType&& err, T&& val) {
  static_cast<Error&>(err) << std::forward<T>(val);
  return std::move(err);
}

/**
 * @brief Creates a new error object and pushes the given one onto the error
 * stack.
 *
 * @tparam ErrorType The resulting error type.
 *
 * @param sub_error
 *  The previous error which is being wrapped.
 * @param loc
 *  Source file location. Defaults to the call site.
 */
template <typename ErrorType>
ErrorType wrap(
  const Error& sub_error,
  const std::experimental::source_location& loc
) {
  static_assert(
    std::is_base_of_v<Error, ErrorType>,
    "Can only wrap with another liblw error class."
  );
  ErrorType err{loc};
  err._set_previous(sub_error._stack);
  return err;
}

template <typename ErrorType>
ErrorType wrap(
  Error&& sub_error,
  const std::experimental::source_location& loc
) {
  static_assert(
    std::is_base_of_v<Error, ErrorType>,
    "Can only wrap with another liblw error class."
  );
  ErrorType err{loc};
  err._set_previous(std::move(sub_error._stack));
  return err;
}

}
