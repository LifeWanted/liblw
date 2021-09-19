#pragma once

#include <cctype>
#include <functional>
#include <iterator>
#include <limits>
#include <string>
#include <string_view>
#include <type_traits>

#include "lw/base/internal/string_concatenate.h"
#include "lw/err/macros.h"

namespace lw {

class CaseInsensitiveHash {
public:
  using is_transparent = void;
  constexpr std::size_t operator()(std::string_view str) const {
    const std::size_t prime = 251;  // Prime around the size of character set.
    const std::size_t modulus = std::numeric_limits<std::size_t>::max();
    std::size_t multiplier = 1;
    std::size_t hash = 0;

    for (auto c : str) {
      auto upper = std::toupper(c);
      hash = (hash + ((upper + 1) * multiplier)) % modulus;
      multiplier = (multiplier * prime) % modulus;
    }

    return hash;
  }
};

// -------------------------------------------------------------------------- //

class CaseInsensitiveEqual {
public:
  using is_transparent = void;
  bool operator()(std::string_view lhs, std::string_view rhs) const {
    if (std::size(lhs) != std::size(rhs)) return false;

    const std::size_t length = std::size(lhs);
    for (std::size_t i = 0; i < length; ++i) {
      if (std::toupper(lhs[i]) != std::toupper(rhs[i]))  return false;
    }
    return true;
  }
};

// -------------------------------------------------------------------------- //

class CaseInsensitiveCompare {
public:
  using is_transparent = void;
  int operator()(std::string_view lhs, std::string_view rhs) const {
    if (lhs.empty() && rhs.empty()) return 0;

    std::size_t i;
    for (i = 0; i < lhs.size() && i < rhs.size(); ++i) {
      const auto l_up = std::toupper(lhs[i]);
      const auto r_up = std::toupper(rhs[i]);
      if (l_up != r_up) return l_up - r_up;
    }
    return lhs.size() - rhs.size();
  }
};

// -------------------------------------------------------------------------- //

class CaseInsensitiveLess {
public:
  using is_transparent = void;
  bool operator()(std::string_view lhs, std::string_view rhs) const {
    return CaseInsensitiveCompare()(lhs, rhs) < 0;
  }
};

// -------------------------------------------------------------------------- //

/**
 * Concatenates one or more arguments into one string.
 *
 * @param args
 *  The values to concatenate into a string. May be of any string-convertible
 *  type.
 *
 * @return
 *  A string containing all the values given, concatenated together.
 */
template <typename... Args>
[[nodiscard]] std::string cat(Args&&... args) {
  std::string result;
  internal::do_concatenate(result, std::forward<Args>(args)...);
  return result;
}

}
