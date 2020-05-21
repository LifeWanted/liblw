#include "lw/flags/format.h"

#include <cmath>
#include <cstdlib>
#include <charconv>
#include <string>
#include <string_view>

#include "lw/err/canonical.h"

namespace lw::cli {
namespace {

constexpr float STRING_ESCAPE_SIZE_FACTOR = 1.2f;

}

template <>
bool parse<bool>(std::string_view value) {
  if (
    value == "y" || value == "Y" || value == "yes" ||
    value == "t" || value == "T" || value == "true" ||
    value == "1"
  ) {
    return true;
  }
  if (
    value == "n" || value == "N" || value == "no" ||
    value == "f" || value == "F" || value == "false" ||
    value == "0"
  ) {
    return false;
  }
  throw InvalidArgument() << "Invalid boolean argument value: " << value;
}

// -------------------------------------------------------------------------- //

#define _LW_NUM_CLI_DEFINE(NUM)                                             \
  template <>                                                               \
  NUM parse<NUM>(std::string_view value) {                                  \
    NUM out;                                                                \
    auto [end, err] = std::from_chars(value.begin(), value.end(), out);     \
    if (end != value.end() || err == std::errc::invalid_argument) {         \
      throw InvalidArgument()                                               \
        << "String \"" << value << "\" is not a valid "#NUM" value.";       \
    } else if (err == std::errc::result_out_of_range) {                     \
      throw InvalidArgument()                                               \
        << "Flag value \"" << value << "\" is too large for "#NUM".";       \
    }                                                                       \
    return out;                                                             \
  }                                                                         \

_LW_NUM_CLI_DEFINE(short);
_LW_NUM_CLI_DEFINE(int);
_LW_NUM_CLI_DEFINE(long int);
_LW_NUM_CLI_DEFINE(long long int);
_LW_NUM_CLI_DEFINE(unsigned short);
_LW_NUM_CLI_DEFINE(unsigned int);
_LW_NUM_CLI_DEFINE(unsigned long int);
_LW_NUM_CLI_DEFINE(unsigned long long int);

#undef _LW_NUM_CLI_DEFINE

// -------------------------------------------------------------------------- //

// TODO: Move these floating point parsers over to std::from_chars once floating
// point support is implemented for that.
template <>
float parse<float>(std::string_view value) {
  char* end = nullptr;
  float out = std::strtof(value.begin(), &end);
  if (end != value.end()) {
    throw InvalidArgument()
      << "String \"" << value << "\" is not a valid float value.";
  } else if (out == HUGE_VALF) {
    throw InvalidArgument()
      << "Flag value \"" << value << "\" is too large for float.";
  }
  return out;
}

template <>
double parse<double>(std::string_view value) {
  char* end = nullptr;
  double out = std::strtod(value.begin(), &end);
  if (end != value.end()) {
    throw InvalidArgument()
      << "String \"" << value << "\" is not a valid double value.";
  } else if (out == HUGE_VAL) {
    throw InvalidArgument()
      << "Flag value \"" << value << "\" is too large for double.";
  }
  return out;
}

template <>
long double parse<long double>(std::string_view value) {
  char* end = nullptr;
  long double out = std::strtold(value.begin(), &end);
  if (end != value.end()) {
    throw InvalidArgument()
      << "String \"" << value << "\" is not a valid long double value.";
  } else if (out == HUGE_VALL) {
    throw InvalidArgument()
      << "Flag value \"" << value << "\" is too large for long double.";
  }
  return out;
}

// -------------------------------------------------------------------------- //

std::string format(std::string_view value) {
  std::string out;
  out.reserve((value.size() * STRING_ESCAPE_SIZE_FACTOR));
  for (const char c : value) {
    if (c == '\\' || c == '"' || std::isspace(c)) {
      out.push_back('\\');
    }
    out.push_back(c);
  }
  return out;
}

template <>
std::string parse<std::string>(std::string_view value) {
  std::string out;
  out.reserve(value.size());
  bool escaped = false;
  for (const char c : value) {
    if (!escaped && c == '\\') {
      escaped = true;
      continue;
    }
    escaped = false;
    out.push_back(c);
  }
  return out;
}

}
