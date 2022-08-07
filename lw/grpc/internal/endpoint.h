#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <unordered_map>

#include "grpcpp/grpcpp.h"
#include "lw/grpc/internal/service_registry.h"
#include "lw/grpc/internal/service_wrapper.h"

namespace lw::grpc::internal {

class BaseEndpoint {
public:
  explicit BaseEndpoint(
    ServiceWrapper& service,
    std::function<void()> runner
  );

  ServiceWrapper& service();
};

template <typename AsyncService, typename Handler>
class Endpoint: public BaseEndpoint {
public:
  using request_type = Handler::request_type;
  using response_type = Handler::response_type;
  using request_method_ptr = void(AsyncService::*)(
    ::grpc::ServerContext*,
    request_type*,
    ::grpc::ServerAsyncResponseWriter<response_type>*,
    ::grpc::CompletionQueue*,
    ::grpc::CompletionQueue*,
    void*
  );

  explicit Endpoint(request_method_ptr request_method):
    BaseEndpoint{
      get_grpc_service<AsyncService>(),
      std::bind(&request, this, request_method, std::placeholder::_1)
    }
  {}

private:
  void request(
    request_method_ptr request_method,
    ::grpc::CompletionQueue* queue
  ) {
    std::uintmax_t handle_id =  ++_handler_counter;
    auto handler = std::make_unique<Handler>();
    service().raw_service->*request_method(
      &handler->grpc_server_context(),
      &handler->grpc_request(),
      &handler->grpc_responder(),
      /*new_call_cq=*/queue,
      /*notification_cq=*/queue,
      handler.get()
    );
  }

  std::uintmax_t _handler_counter = 0;
  std::unordered_map<std::uintmax_t, std::unique_ptr<Handler>> _handlers;
};

}
