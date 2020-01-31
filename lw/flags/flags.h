#pragma once

#include <cstdlib>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>

#include "lw/err/canonical.h"

#define LW_FLAG(type, name, default_value, description)               \
  namespace lw::flags {                                               \
    ::lw::Flag<type> name{#type, #name, default_value, description};  \
  }                                                                   \

#define LW_DECLARE_FLAG(type, name) \
  namespace lw::flags { extern ::lw::Flag<type> name; }

namespace lw {
namespace impl {

template <typename T>
inline constexpr bool is_bool = std::is_same_v<T, bool>;

template <typename T>
inline constexpr bool is_integer = std::is_integral_v<T> && !is_bool<T>;

template <typename T>
inline constexpr bool is_float = std::is_floating_point_v<T>;

template <typename T>
inline constexpr bool is_string = std::is_same_v<T, std::string>;

struct FlagHelper {
  template <typename Bool, std::enable_if_t<is_bool<Bool>>* = nullptr>
  static std::string format(bool value) { return value ? "true" : "false"; }

  template <typename Bool, std::enable_if_t<is_bool<Bool>>* = nullptr>
  static bool parse(std::string_view value) {
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

  // ------------------------------------------------------------------------ //

  template <typename Integer, std::enable_if_t<is_integer<Integer>>* = nullptr>
  static std::string format(Integer value) { return std::to_string(value); }

  template <typename Integer, std::enable_if_t<is_integer<Integer>>* = nullptr>
  static Integer parse(std::string_view value) {
    if constexpr (std::is_unsigned<Integer>::value) {
      return std::strtoull(value.begin(), nullptr, 10);
    } else {
      return std::strtoll(value.begin(), nullptr, 10);
    }
  }

  // ------------------------------------------------------------------------ //

  template <typename Float, std::enable_if_t<is_float<Float>>* = nullptr>
  static std::string format(Float value) { return std::to_string(value); }

  template <typename Float, std::enable_if_t<is_float<Float>>* = nullptr>
  static Float parse(std::string_view value) {
    if constexpr (std::is_same<Float, float>::value) {
      return std::strtof(value.begin(), nullptr);
    } else if constexpr (std::is_same<Float, double>::value) {
      return std::strtod(value.begin(), nullptr);
    } else {
      return std::strtold(value.begin(), nullptr);
    }
  }

  // ------------------------------------------------------------------------ //

  // TODO: Wrap value in quotes and escape internal quotes.
  template <typename String, std::enable_if_t<is_string<String>>* = nullptr>
  static std::string format(const String& value) { return value; }

  template <typename String, std::enable_if_t<is_string<String>>* = nullptr>
  static String parse(std::string_view value) {
    return {value.begin(), value.end()};
  }
};

}

// -------------------------------------------------------------------------- //


class FlagBase;

/**
 * Returns true if a flag with the name given has been defined by the
 * application. Any underscores in the name are converted to hyphens before
 * checking.
 */
bool flags_exists(std::string_view flag_name);

/**
 * Sets the value of the named flag using either the optional value or the next
 * argument.
 *
 * @return
 *  True if the next argument in `rest_args` was used as the value for this
 *  argument. Otherwise, false is returned.
 */
bool flags_cli_set(
  std::string_view flag_name,
  std::optional<std::string_view> value,
  const char* const* rest_args,
  int argc
);

// -------------------------------------------------------------------------- //

class FlagBase {
public:
  explicit FlagBase(
    std::string_view type_name,
    std::string_view name,
    std::string_view description
  );
  FlagBase(const FlagBase&) = delete;
  FlagBase& operator=(const FlagBase&) = delete;

  const std::string& type_name()    const { return _type_name; }
  const std::string& name()         const { return _name; }
  const std::string& description()  const { return _description; }

  virtual std::string default_value_string() const = 0;
  virtual void parse_value(std::string_view value_str) = 0;

private:
  std::string _type_name;
  std::string _name;
  std::string _description;
};

template <typename T>
class Flag: public FlagBase {
public:
  static_assert(
    !std::is_same_v<T, char*> && !std::is_same_v<T, const char*> &&
    !std::is_same_v<T, std::string_view>,
    "String flags must use std::string."
  );
  static_assert(
    std::is_same_v<std::remove_reference_t<std::remove_cv_t<T>>, T>,
    "Flags must use non-const, non-volatile, non-reference types."
  );
  static_assert(
    std::is_same_v<std::remove_pointer_t<T>, T>,
    "Flags types cannot be pointers."
  );

  explicit Flag(
    std::string_view type_name,
    std::string_view name,
    const T& default_value,
    std::string_view description
  ):
    FlagBase{type_name, name, description},
    _default{default_value}
  {}

  const T& value() const {
    return _value ? *_value : _default;
  }

  operator const T&() const {
    return value();
  }

  template <typename U>
  bool operator==(U&& other) const {
    return value() == other;
  }

  template <typename U>
  bool operator!=(U&& other) const {
    return value() != other;
  }

  std::string default_value_string() const override {
    return ::lw::impl::FlagHelper::format<T>(_default);
  }

  void parse_value(std::string_view value_str) override {
    set_value(::lw::impl::FlagHelper::parse<T>(value_str));
  }

  template<typename U>
  void set_value(U&& value) {
    _value = std::forward<U>(value);
  }

  template<typename U>
  Flag& operator=(U&& value) {
    set_value(std::forward<U>(value));
    return *this;
  }

private:
  const T _default;
  std::optional<T> _value;
};

/**
 * This namespace is reserved for registered flags. Flags defined with `LW_FLAG`
 * or declared with `LW_DECLARE_FLAG` show up in the `lw::flags` namespace.
 */
namespace flags {}

}
