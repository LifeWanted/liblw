#pragma once

#include <string_view>

namespace lw::net {

class HttpHandler {
public:
  explicit HttpHandler(std::string_view route): _route{route} {}

  std::string_view route() const { return _route; }

private:
  std::string _route;
};

}
