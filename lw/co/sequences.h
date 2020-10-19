#pragma once

#include <type_traits>

#include "lw/co/generator.h"

namespace lw::co {

template <typename Iterator>
Generator<std::decay_t<Iterator>> range(Iterator begin, Iterator end) {
  using OutType = std::decay_t<Iterator>;
  if constexpr (std::is_arithmetic_v<Iterator> || std::is_pointer_v<Iterator>) {
    if (end < begin) {
      for (OutType i = begin; i != end; --i) {
        co_yield i;
      }
      co_return;
    }
  }
  for (OutType i = begin; i != end; ++i) {
    co_yield i;
  }
}

template <typename Iterator, typename Numeric>
Generator<std::decay_t<Iterator>> range(
  Iterator begin, Iterator end, Numeric increment
) {
  using OutType = std::decay_t<Iterator>;
  if constexpr (std::is_arithmetic_v<Iterator> || std::is_pointer_v<Iterator>) {
    if (end < begin) {
      for (OutType i = begin; i > end; i -= increment) {
        co_yield i;
      }
      co_return;
    }
  }
  for (OutType i = begin; i < end; i += increment) {
    co_yield i;
  }
}

}
