#pragma once

#include "lw/http/http.h"
#include "lw/net/tls_router.h"

namespace lw {

using HttpsRouter =
  ::lw::net::TLSRouter<::lw::HttpRouter>;

}
