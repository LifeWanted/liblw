#pragma once

#include <cstdlib>
#include <string>
#include <string_view>

namespace lw::cli {

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

}
