#pragma once

#include <concepts>
#include <cstdint>

#include "lw/co/concepts.h"
#include "lw/memory/buffer.h"

namespace lw::io {

template <typename T>
concept CoReadable = requires(const T& a) {
  { a.eof() } -> std::convertible_to<bool>;
  { a.good() } -> std::convertible_to<bool>;
} && requires(T& a, Buffer b) {
  { a.read(b) } -> co::ValueAwaitable<std::size_t>;
};

template <typename T>
concept CoWritable = requires(const T& a) {
  { a.good() } -> std::convertible_to<bool>;
} && requires(T& a, const Buffer b) {
  { a.write(b) } -> co::ValueAwaitable<std::size_t>;
};

template <typename T>
concept CoReadWritable = CoReadable<T> && CoWritable<T>;

}
