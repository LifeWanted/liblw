#pragma once

#include <cstdint>
#include <memory>
#include <set>

#include "grpcpp/grpcpp.h"
#include "lw/co/future.h"
#include "lw/grpc/internal/service_wrapper.h"

namespace lw::grpc::internal {

/**
 * Wraps the `::grpc::Server` interface in a co-routine friendly API.
 */
class ServerWrapper {
public:
  ServerWrapper(std::uint16_t port);
  ~ServerWrapper();

  /**
   * Attaches the given service and endpoint runners.
   */
  void attach_service(ServiceWrapper* service);

  /**
   * Returns true if the gRPC server is running.
   */
  bool running() const { return _server != nullptr; }

  /**
   * Shuts down the server.
   *
   * TODO: Drain the server before closing it.
   */
  void close();

  bool try_close();

  void force_close();

  /**
   * Creates and connects the gRPC service. This method executes until `close`
   * is called.
   */
  co::Future<void> run();

private:
  std::unique_ptr<::grpc::ServerBuilder> _builder;
  std::unique_ptr<::grpc::ServerCompletionQueue> _queue;
  std::unique_ptr<::grpc::Server> _server;
  std::set<ServiceWrapper*> _services;
};

}
