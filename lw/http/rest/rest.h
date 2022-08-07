#pragma once

#include "lw/http/rest/rest_handler.h"
#include "lw/http/rest/rest_router.h"

#define LW_REGISTER_REST_HANDLER(HandlerClass, route) \
  ::lw::RestRoute rest_route_ ## HandlerClass{        \
    ::lw::RestHandlerFactory<HandlerClass>{route}     \
  }
