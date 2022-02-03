#pragma once
/**
 * @file
 * Define a new command-line flag. The value will be stored as a `Flag<type>`
 * in the namespace `lw::flags`.
 *
 * Example:
 * ```cpp
 * LW_FLAG(int, max_foo, 1, "Maximum foo to bar.");
 *
 * int main() {
 *   const int FOO_LIMIT = lw::flags::max_foo;
 *   std::cout << FOO_LIMIT << std::endl;
 * }
 * ```
 */

#include <cstdlib>
#include <optional>
#include <ostream>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>

#include "lw/err/canonical.h"
#include "lw/flags/format.h"

#define LW_FLAG(type, name, default_value, description)                   \
  namespace lw::flags {                                                   \
    ::lw::cli::Flag<type> name{#type, #name, default_value, description}; \
  }                                                                       \

#define LW_DECLARE_FLAG(type, name) \
  namespace lw::flags { extern ::lw::cli::Flag<type> name; }

namespace lw::cli {

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
 * @param flag_name
 *  The name of the flag, what appears between the `--` and `=`.
 * @param value
 *  Any text value parsed after the first `=` if there is one.
 * @param rest_args
 *  A pointer to the next argument after the one being set.
 * @param argc
 *  The number of arguments after this one.
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

/**
 * Formats all of the flags with their type, defaults, and description suitable
 * for printing to a terminal window.
 *
 * @param out
 *  An output stream to write the formatted flag descriptions.
 */
void print_flags(std::ostream& out);

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
    return format(_default);
  }

  void parse_value(std::string_view value_str) override {
    set_value(parse<T>(value_str));
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

}

/**
 * This namespace is reserved for registered flags. Flags defined with `LW_FLAG`
 * or declared with `LW_DECLARE_FLAG` show up in the `lw::flags` namespace.
 */
namespace lw::flags {}
