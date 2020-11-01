#pragma once

#include <map>
#include <string>
#include <string_view>

#include "lw/base/strings.h"

namespace lw::http {

typedef std::map<
  std::string_view,
  std::string_view,
  CaseInsensitiveLess
> HeadersView;

typedef std::map<std::string, std::string, CaseInsensitiveLess> Headers;

}
