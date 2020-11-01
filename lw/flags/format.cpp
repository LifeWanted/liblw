#include "lw/flags/format.h"

#include <cmath>
#include <cstdlib>
#include <charconv>
#include <chrono>
#include <string>
#include <string_view>
#include <system_error>

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

// -------------------------------------------------------------------------- //

template <>
std::chrono::nanoseconds parse<std::chrono::nanoseconds>(
  std::string_view value
) {
  std::chrono::nanoseconds::rep count;
  const auto [num_end, ec] = std::from_chars(value.begin(), value.end(), count);
  if (num_end == value.end()) {
    throw InvalidArgument()
      << "Duration value \"" << value << "\" missing time unit suffix.";
  } else if (num_end == value.begin() || ec == std::errc::invalid_argument) {
    throw InvalidArgument()
      << "Duration value \"" << value << "\" could not be parsed.";
  } else if (ec == std::errc::result_out_of_range) {
    throw InvalidArgument()
      << "Duration value \"" << value << "\" too large to be represented.";
  }

  std::string_view units{num_end, static_cast<uint64_t>(value.end() - num_end)};
  if (units == "y") {
    return std::chrono::years(count);
  } else if (units == "d") {
    return std::chrono::days(count);
  } else if (units == "h") {
    return std::chrono::hours(count);
  } else if (units == "m") {
    return std::chrono::minutes(count);
  } else if (units == "s") {
    return std::chrono::seconds(count);
  } else if (units == "ms") {
    return std::chrono::milliseconds(count);
  } else if (units == "us") {
    return std::chrono::microseconds(count);
  } else if (units == "ns") {
    return std::chrono::nanoseconds(count);
  } else {
    throw InvalidArgument()
      << "Unknown time units \"" << units << "\" in value " << value;
  }
}

}
