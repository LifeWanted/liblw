#pragma once

#include <concepts>
#include <iterator>

namespace lw {

template <typename T, typename V>
concept IterableAs =
  requires(const T& a) {
    { *std::begin(a) } -> std::same_as<const V&>;
  } ||
  requires(const T& a) {
    { a.begin() } -> std::same_as<const V&>;
  };

template <typename T>
concept ForwardIterable =
  requires(const T& a) {
    { std::begin(a) } -> std::incrementable;
    { std::end(a) };
  } ||
  requires(const T& a) {
    { a.begin() } -> std::incrementable;
    { a.end() };
    { a.begin() != a.end() } -> std::convertible_to<bool>;
  };

template <typename T, typename V>
concept ForwardIterableAs = ForwardIterable<T> && IterableAs<T, V>;

}
