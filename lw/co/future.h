#pragma once

#include <exception>
#include <future>

namespace lw::co {

/**
 * Creates a future that is already resolved with the given value.
 */
template <typename T>
std::future<T> make_future(T&& value) {
  std::promise<T> promise;
  promise.set_value(std::forward<T>(value));
  return promise.get_future();
}

/**
 * Create a future that is rejected with the given exception.
 */
template <typename T>
std::future<T> make_future(std::exception_ptr err) {
  std::promise<T> promise;
  promise.set_exception(err);
  return promise.get_future();
}

}
