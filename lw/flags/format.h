#pragma once

#include <chrono>
#include <cstdlib>
#include <string>
#include <string_view>
#include <type_traits>

namespace lw::cli {
namespace impl {

/**
 * Determines if the given duration can be losslessly converted to new units.
 *
 * @return
 *  True if `duration` can be cast to the new units and back again
 *  without losing any resolution.
 */
template <typename DestinationUnits, typename Rep, typename Period>
constexpr bool can_convert_duration(
  const std::chrono::duration<Rep, Period>& duration
) {
  typedef DestinationUnits destination_units;
  typedef std::chrono::duration<Rep, Period> source_units;
  if constexpr (std::is_same_v<destination_units, source_units>) {
    return true;
  }

  const source_units converted_duration =
    std::chrono::duration_cast<source_units>(
      std::chrono::duration_cast<destination_units>(duration)
    );
  return converted_duration == duration;
}

}

inline std::string format(bool value) { return value ? "true" : "false"; }
inline std::string format(short value) { return std::to_string(value); }
inline std::string format(int value) { return std::to_string(value); }
inline std::string format(long int value) { return std::to_string(value); }
inline std::string format(long long int value) { return std::to_string(value); }
inline std::string format(unsigned short value) {
  return std::to_string(value);
}
inline std::string format(unsigned int value) { return std::to_string(value); }
inline std::string format(unsigned long int value) {
  return std::to_string(value);
}
inline std::string format(unsigned long long int value) {
  return std::to_string(value);
}
inline std::string format(float value) { return std::to_string(value); }
inline std::string format(long double value) { return std::to_string(value); }
std::string format(std::string_view value);
template <std::size_t Size>
inline std::string format(const char value[Size]) {
  return format(std::string_view{(const char*)value, Size});
}
inline std::string format(const char* value) {
  return format(std::string_view{value});
}

template <typename Rep, typename Period>
std::string format(const std::chrono::duration<Rep, Period>& duration) {
  using namespace std::chrono;
  if (impl::can_convert_duration<years>(duration)) {
    return format(duration_cast<years>(duration).count()) + "y";
  } else if (impl::can_convert_duration<days>(duration)) {
    return format(duration_cast<days>(duration).count()) + "d";
  } else if (impl::can_convert_duration<hours>(duration)) {
    return format(duration_cast<hours>(duration).count()) + "h";
  } else if (impl::can_convert_duration<minutes>(duration)) {
    return format(duration_cast<minutes>(duration).count()) + "m";
  } else if (impl::can_convert_duration<seconds>(duration)) {
    return format(duration_cast<seconds>(duration).count()) + "s";
  } else if (impl::can_convert_duration<milliseconds>(duration)) {
    return format(duration_cast<milliseconds>(duration).count()) + "ms";
  } else {
    return format(duration_cast<nanoseconds>(duration).count()) + "ns";
  }
}

// -------------------------------------------------------------------------- //

template <typename T>
T parse(std::string_view value);
template <>
bool parse<bool>(std::string_view value);
template <>
short parse<short>(std::string_view value);
template <>
int parse<int>(std::string_view value);
template <>
long int parse<long int>(std::string_view value);
template <>
long long int parse<long long int>(std::string_view value);
template <>
unsigned short parse<unsigned short>(std::string_view value);
template <>
unsigned int parse<unsigned int>(std::string_view value);
template <>
unsigned long int parse<unsigned long int>(std::string_view value);
template <>
unsigned long long int parse<unsigned long long int>(std::string_view value);
template <>
float parse<float>(std::string_view value);
inline std::string format(double value) { return std::to_string(value); }
template <>
double parse<double>(std::string_view value);
template <>
long double parse<long double>(std::string_view value);
template <>
std::string parse<std::string>(std::string_view value);
template <>
std::chrono::nanoseconds parse<std::chrono::nanoseconds>(
  std::string_view value
);

}
