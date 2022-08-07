#include "lw/grpc/server.h"

#include <memory>

#include "grpcpp/grpcpp.h"
#include "lw/grpc/internal/grpc_wrapper.h"
#include "lw/grpc/internal/service_registry.h"

namespace lw::grpc {

Server::Server(const Options& options):
  _grpc_wrapper{std::make_unique<internal::GrpcWrapper>(options.port)}
{}

void Server::attach_services() {
  for (
    ::grpc::AsyncGenericService* service : internal::get_registered_services()
  ) {
    _grpc_wrapper->attach_service(service);
  }
}

bool Server::running() const {
  return _grpc_wrapper->running();
}

void Server::close() {
  _grpc_wrapper->close();
}

bool Server::try_close() {
  return _grpc_wrapper->try_close();
}

void Server::force_close() {
  _grpc_wrapper->force_close();
}

void Server::run() {
  _grpc_wrapper->run();
}

}
