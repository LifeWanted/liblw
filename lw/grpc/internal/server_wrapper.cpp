#include "lw/grpc/internal/server_wrapper.h"

#include <cstdint>
#include <memory>
#include <string_view>
#include <vector>

#include "grpcpp/grpcpp.h"
#include "lw/base/strings.h"
#include "lw/co/future.h"
#include "lw/err/canonical.h"
#include "lw/err/macros.h"
#include "lw/log/log.h"

namespace lw::grpc::internal {

constexpr std::string_view GRPC_HOST = "0.0.0.0";

ServerWrapper::ServerWrapper(std::uint16_t port):
  _builder{std::make_unique<::grpc::ServerBuilder>()}
{
  _builder->AddListeningPort(
    cat(GRPC_HOST, ':', port),
    // TODO: Make this configurable.
    ::grpc::InsecureServerCredentials()
  );
}

void ServerWrapper::attach_service(ServiceWrapper* service) {
  LW_CHECK_NULL(service);
  LW_CHECK_NULL_INTERNAL(_builder);
  _builder->RegisterService(service->raw_service());
  _services.insert(service);
}

void ServerWrapper::close() {
  if (_server) {
    _server->Shutdown();
    _queue->Shutdown();
    _server = nullptr;
  }
}

ServerWrapper::~ServerWrapper() {
  if (running()) close();
}

bool ServerWrapper::try_close() {
  throw Unimplemented() << "ServerWrapper::try_close is unimplemented.";
}

void ServerWrapper::force_close() {
    throw Unimplemented() << "ServerWrapper::force_close is unimplemented.";
}

co::Future<void> ServerWrapper::run() {
  LW_CHECK_NULL_INTERNAL(_builder);
  _queue = _builder->AddCompletionQueue();
  _server = _builder->BuildAndStart();
  _builder = nullptr;

  log(INFO) << "gRPC server starting " << _services.size() << " services.";
  std::vector<co::Future<void>> futures;
  futures.reserve(_services.size());
  for (ServiceWrapper* service : _services) {
    futures.push_back(service->run(_queue.get()));
  }
  co_await co::all_void(futures.begin(), futures.end());
  log(INFO) << "gRPC services finished, exiting.";
}

}
