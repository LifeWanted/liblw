#pragma once

#include <functional>
#include <list>

#include "grpcpp/grpcpp.h"
#include "lw/co/future.h"

namespace lw::grpc::internal {

class ServerWrapper;

using RequestRunner =
  std::function<co::Future<void>(::grpc::ServerCompletionQueue*)>;

class ServiceWrapper {
public:
  explicit ServiceWrapper(::grpc::Service* service):
    _service{service}
  {}

  /**
   * Returns a pointer to the underlying gRPC service object.
   */
  ::grpc::Service* raw_service() const { return _service; }

  /**
   * Adds a service endpoint runner to the service.
   *
   * Runners are all called concurrently when the service starts.
   */
  template <typename Func>
  void add_runner(Func&& runner) {
    _runners.push_back(std::forward<Func>(runner));
  }

  /**
   * Executes all service endpoint runners concurrently. The future resolves
   * when all the endpoint runners resolve, which is likely service shutdown.
   */
  co::Future<void> run(::grpc::ServerCompletionQueue* queue);

private:
  ::grpc::Service* _service;
  std::list<RequestRunner> _runners;
};

}
