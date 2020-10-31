#pragma once

#include <experimental/source_location>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <type_traits>

#include "lw/io/stream/buffer.h"

namespace lw {

/**
 * Base class for all exceptions thrown within liblw.
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
    return _buffer.c_str();
  }
  const std::experimental::source_location& where() const noexcept {
    return _location;
  }
  std::string_view what_string() const noexcept {
    return _buffer.string();
  }

protected:
  Error(std::string_view name, const std::experimental::source_location& loc);

private:
  template <typename T>
  friend Error& operator<<(Error&, T&&);
  template <typename T>
  friend Error&& operator<<(Error&&, T&&);

  std::experimental::source_location _location;
  io::stream::StringBuffer _buffer;
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

}
