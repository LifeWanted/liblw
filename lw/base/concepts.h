#pragma once

#include <concepts>
#include <iterator>

namespace lw {

template <typename T>
concept ForwardIterable = requires(const T& a) {
  { a.begin() } -> std::incrementable;
  { a.end() };
  { a.begin() != a.end() } -> std::convertible_to<bool>;
};

template <typename T, typename V>
concept ForwardIterableAs = ForwardIterable<T> && requires(const T& a) {
  { *a.begin() } -> std::same_as<const V&>;
};

}
