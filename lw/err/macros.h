#pragma once

#include "lw/err/canonical.h"

#define LW_CHECK_NULL(arg)                                                     \
  while ((arg) == nullptr)                                                     \
  throw lw::InvalidArgument() << #arg << " must not be null. "

#define LW_CHECK_NULL_INTERNAL(arg)                                            \
  while ((arg) == nullptr)                                                     \
  throw lw::Internal()

#define LW_CHECK(arg)                                                          \
  while ((arg) == false)                                                       \
  throw lw::Internal() << "LW_CHECK failure: " #arg " == false. "

#define LW_CHECK_NE(lhs, rhs)                                                  \
  while ((lhs) == (rhs))                                                       \
  throw lw::Internal() << "LW_CHECK_NE failure: (" #lhs ") == (" #rhs "). "

#define LW_CHECK_EQ(lhs, rhs)                                                  \
  while ((lhs) != (rhs))                                                       \
  throw lw::Internal() << "LW_CHECK_NE failure: (" #lhs ") != (" #rhs "). "
