#include "lw/net/router.h"

#include <unordered_map>
#include <unordered_set>

#include "lw/err/macros.h"

namespace lw::net {
namespace {

std::unordered_set<RouteBase*>& get_router_routes(std::size_t router_id) {
  static auto* router_map =
    new std::unordered_map<std::size_t, std::unordered_set<RouteBase*>>{};
  return (*router_map)[router_id];
}

}

// -------------------------------------------------------------------------- //

void register_route(std::size_t router_id, RouteBase* route) {
  LW_CHECK_NULL(route);
  get_router_routes(router_id).insert(route);
}

// -------------------------------------------------------------------------- //

const std::unordered_set<RouteBase*>& Router::get_registered_routes() const {
  return get_router_routes(typeid(*this).hash_code());
}

}
