#pragma once

#include "lw/err/cannonical.h"

#define LW_CHECK_NULL(arg)  \
  if ((arg) == nullptr)     \
    throw lw::InvalidArgument() << #arg << " must not be null. "
