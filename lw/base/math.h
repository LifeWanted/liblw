#pragma once

#include <concepts>

namespace lw {

/**
 * Linear interpolation between integral points.
 *
 * @param a
 *  Low end of the range to interpolate through.
 * @param b
 *  High end of the range to interpolate through.
 * @param x
 *  Distance into the range to go, from 0.0 to 1.0.
 *
 * @return
 *  The integer point interpolated between `a` and `b`.
 */
template <std::integral T>
constexpr T lerp(T a, T b, double x) {
  return a + (x * static_cast<double>(b - a));
}

/**
 * Linear interpolation between floating-point points.
 *
 * @param a
 *  Low end of the range to interpolate through.
 * @param b
 *  High end of the range to interpolate through.
 * @param x
 *  Distance into the range to go, from 0.0 to 1.0.
 *
 * @return
 *  The floating-point point interpolated between `a` and `b`.
 */
template <std::floating_point T>
constexpr T lerp(T a, T b, T x) {
  return a + (x * (b - a));
}

}
