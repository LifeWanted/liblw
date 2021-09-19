#pragma once

#include "lw/err/canonical.h"

#define _LW_UNWRAP_STRING_IMPL(x) #x
#define _LW_UNWRAP_STRING(x) _LW_UNWRAP_STRING_IMPL x

#define LW_CHECK_NULL(arg)  \
  while ((arg) == nullptr)  \
    throw lw::InvalidArgument() << #arg << " must not be null. "

#define LW_CHECK_NULL_INTERNAL(arg) \
  while ((arg) == nullptr) throw lw::Internal()

#define _LW_CHECK_GT_TYPED_IMPL(rhs, lhs, ErrorClass)                     \
  while (!((rhs) > (lhs)))                                                \
    throw ErrorClass()                                                    \
      << "Expected `" _LW_UNWRAP_STRING(rhs) "` (" << rhs                 \
      << ") > `" _LW_UNWRAP_STRING(lhs) "` (" << lhs << ") to be true. "

#define LW_CHECK_GT_TYPED(rhs, lhs, ErrorClass) \
  _LW_CHECK_GT_TYPED_IMPL((rhs), (lhs), ErrorClass)

#define LW_CHECK_GT(rhs, lhs) \
  _LW_CHECK_GT_TYPED_IMPL((rhs), (lhs), ::lw::FailedPrecondition)

#define _LW_CHECK_GTE_TYPED_IMPL(rhs, lhs, ErrorClass)                    \
  while (!((rhs) >= (lhs)))                                               \
    throw ErrorClass()                                                    \
      << "Expected `" _LW_UNWRAP_STRING(rhs) "` (" << rhs                 \
      << ") >= `" _LW_UNWRAP_STRING(lhs) "` (" << lhs << ") to be true. "

#define LW_CHECK_GTE_TYPED(rhs, lhs, ErrorClass) \
  _LW_CHECK_GTE_TYPED_IMPL((rhs), (lhs), ErrorClass)

#define LW_CHECK_GTE(rhs, lhs) \
  _LW_CHECK_GTE_TYPED_IMPL((rhs), (lhs), ::lw::FailedPrecondition)
