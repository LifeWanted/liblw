#pragma once

#include "lw/err/canonical.h"

#define LW_CHECK_NULL(arg)  \
  while ((arg) == nullptr)  \
    throw lw::InvalidArgument() << #arg << " must not be null. "

#define LW_CHECK_NULL_INTERNAL(arg) \
  while ((arg) == nullptr) throw lw::Internal()
