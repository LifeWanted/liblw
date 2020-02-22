#pragma once

#include <cctype>
#include <functional>
#include <iterator>
#include <limits>
#include <string_view>
#include <type_traits>

#include "lw/err/macros.h"

namespace lw {

template <typename StringT>
class CaseInsensitiveHash {
public:
  std::size_t operator()(const StringT& str) const {
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

template <>
class CaseInsensitiveHash<const char*> {
public:
  std::size_t operator()(const char* str) const {
    LW_CHECK_NULL(str);

    const std::size_t prime = 251;  // Prime around the size of character set.
    const std::size_t modulus = std::numeric_limits<std::size_t>::max();
    std::size_t multiplier = 1;
    std::size_t hash = 0;

    for (const char* c = str; *c; ++c) {
      auto upper = std::toupper(*c);
      hash = (hash + ((upper + 1) * multiplier)) % modulus;
      multiplier = (multiplier * prime) % modulus;
    }

    return hash;
  }
};

// -------------------------------------------------------------------------- //

template <typename StringT>
class CaseInsensitiveEqual {
public:
  bool operator()(const StringT& lhs, const StringT& rhs) const {
    if (std::size(lhs) != std::size(rhs)) return false;

    const std::size_t length = std::size(lhs);
    for (std::size_t i = 0; i < length; ++i) {
      if (std::toupper(lhs[i]) != std::toupper(rhs[i]))  return false;
    }
    return true;
  }
};

template <>
class CaseInsensitiveEqual<const char*> {
public:
  bool operator()(const char* lhs, const char* rhs) const {
    LW_CHECK_NULL(lhs);
    LW_CHECK_NULL(rhs);

    for (; *lhs && *rhs; ++lhs, ++rhs) {
      if (std::toupper(*lhs) != std::toupper(*rhs))  return false;
    }
    return *lhs == *rhs; // Both reached the end at the same time.
  }
};

}
