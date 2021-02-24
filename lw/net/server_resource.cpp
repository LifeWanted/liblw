#include "lw/net/server_resource.h"

#include <memory>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <variant>

#include "lw/co/future.h"
#include "lw/err/canonical.h"

namespace lw {
namespace {

std::unordered_map<std::type_index, std::unique_ptr<ServerResourceFactoryBase>>&
get_server_resources() {
  static auto* resource_map = new std::unordered_map<
    std::type_index,
    std::unique_ptr<ServerResourceFactoryBase>
  >{};
  return *resource_map;
}

}

bool register_server_resource(
  const std::type_info& resource_info,
  std::unique_ptr<ServerResourceFactoryBase> factory
) {
  get_server_resources().insert({{resource_info}, std::move(factory)});
  return true;
}

co::Future<std::unique_ptr<ServerResourceBase>> construct_server_resource(
  const std::type_info& resource_info,
  ServerResourceContext& context
) {
  std::type_index resource_idx{resource_info};
  if (!get_server_resources().contains(resource_idx)) {
    throw NotFound()
      << "Server resource " << resource_info.name()
      << " has no factory registered.";
  }
  return (*get_server_resources()[resource_idx])(context);
}

}
