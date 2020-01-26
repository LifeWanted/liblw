#pragma once

#include <experimental/source_location>
#include <string_view>

#include "lw/err/error.h"

namespace lw {

#define _LW_DEFINE_ERROR(name)                        \
  class name : public ::lw::Error {                   \
  public:                                             \
    name(                                             \
      const std::experimental::source_location& loc = \
        std::experimental::source_location::current() \
    ):                                                \
      ::lw::Error(#name, loc)                         \
    {}                                                \
  }

_LW_DEFINE_ERROR(InvalidArgument);

#undef _LW_DEFINE_ERROR

}
