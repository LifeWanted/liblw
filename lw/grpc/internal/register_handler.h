#pragma once

#include <functional>
#include <memory>
#include <type_info>

namespace grpc {
class Service;
}

namespace lw::grpc {

class BaseHandler;

namespace internal {

bool has_service(const std::type_info& service_type);
void add_service(
  const std::type_info& service_type,
  std::unique_ptr<::grpc::Service> service
);

void insert_registration(
  const std::type_info& service_type,
  ??? endpoint_hook,
  std::function<std::unique_ptr<BaseHandler>()> handler_factory
);

template <typename Service, typename Handler, typename Endpoint>
void register_handler(Endpoint* endpoint_hook) {
  std::type_info service_type = typeid(Service);
  if (!has_service(service_type)) {
    add_service(service_type, std::make_unique<Service>());
  }

  insert_registration(
    service_type,
    endpoint_hook,
    []() { return std::make_unique<Handler>(); }
  );
}

}
}
