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

_LW_DEFINE_ERROR(Aborted);
_LW_DEFINE_ERROR(AlreadyExists);
_LW_DEFINE_ERROR(Cancelled);
_LW_DEFINE_ERROR(DeadlineExceeded);
_LW_DEFINE_ERROR(FailedPrecondition);
_LW_DEFINE_ERROR(Internal);
_LW_DEFINE_ERROR(InvalidArgument);
_LW_DEFINE_ERROR(NotFound);
_LW_DEFINE_ERROR(OutOfRange);
_LW_DEFINE_ERROR(PermissionDenied);
_LW_DEFINE_ERROR(ResourceExhausted);
_LW_DEFINE_ERROR(Unavailable);
_LW_DEFINE_ERROR(Unimplemented);

#undef _LW_DEFINE_ERROR

}
