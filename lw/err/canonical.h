#pragma once

#include <experimental/source_location>
#include <string_view>

#include "lw/err/error.h"

namespace lw {

class CanonicalError: public Error {
public:
  enum Code {
    Aborted,
    AlreadyExists,
    Cancelled,
    DeadlineExceeded,
    FailedPrecondition,
    Internal,
    InvalidArgument,
    NotFound,
    OutOfRange,
    PermissionDenied,
    ResourceExhausted,
    Unavailable,
    Unimplemented
  };

  Code code() const { return _code; }

protected:
  CanonicalError(
    Code code,
    std::string_view name,
    const std::experimental::source_location& loc
  ):
    Error{name, loc},
    _code{code}
  {}

private:
  Code _code;
};

#define _LW_DEFINE_ERROR(name)                                \
  class name : public CanonicalError {                        \
  public:                                                     \
    name(                                                     \
      const std::experimental::source_location& loc =         \
        std::experimental::source_location::current()         \
    ):                                                        \
      CanonicalError(CanonicalError::Code:: name, #name, loc) \
    {}                                                        \
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
