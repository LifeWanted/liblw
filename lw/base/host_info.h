#pragma once

#include <cstdint>
#include <concepts>

namespace lw {

inline bool is_big_endian_byte_order() {
  union {
    std::uint16_t test;
    struct {
      std::uint8_t little;
      std::uint8_t big;
    } endian;
  } byte_order = {.test = 1};
  return byte_order.endian.big == 1;
}

template <std::integral T>
constexpr T flip_byte_order(T value) {
  static_assert(
    sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4 || sizeof(T) == 8,
    "Integral type must have power of 2 bytes."
  );

  if constexpr (sizeof(T) == 1) {
    return value;
  } else if constexpr (sizeof(T) == 2) {
    return ((0x00ff & value) << 8) | ((0xff00 & value) >> 8);
  } else if constexpr (sizeof(T) == 4) {
    return (
      ((0x000000ff & value) << 24) |
      ((0x0000ff00 & value) << 8) |
      ((0x00ff0000 & value) >> 8) |
      ((0xff000000 & value) >> 24)
    );
  } else if constexpr (sizeof(T) == 8) {
    return (
      ((0x00000000000000ff & value) << 56) |
      ((0x000000000000ff00 & value) << 40) |
      ((0x0000000000ff0000 & value) << 24) |
      ((0x00000000ff000000 & value) << 8) |
      ((0x000000ff00000000 & value) >> 8) |
      ((0x0000ff0000000000 & value) >> 24) |
      ((0x00ff000000000000 & value) >> 40) |
      ((0xff00000000000000 & value) >> 56)
    );
  }
}

template <std::integral T>
T host_to_big_endian(T value) {
  if (is_big_endian_byte_order()) return value;
  return flip_byte_order(value);
}

}
