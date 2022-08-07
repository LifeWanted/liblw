#include "lw/grpc/internal/server_wrapper.h"

#include <cstdint>
#include <memory>
#include <string_view>
#include <vector>

#include "grpcpp/grpcpp.h"
#include "lw/base/strings.h"
#include "lw/co/future.h"
#include "lw/co/scheduler.h"
#include "lw/co/task.h"
#include "lw/err/canonical.h"
#include "lw/err/macros.h"
#include "lw/grpc/internal/queue_processor.h"
#include "lw/log/log.h"
#include "lw/thread/thread.h"

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
  // _builder = nullptr;

  // Initialize the services on this thread so they attach to this scheduler.
  log(INFO) << "gRPC server starting " << _services.size() << " services.";
  std::vector<co::Future<void>> service_futures;
  service_futures.reserve(_services.size());
  for (ServiceWrapper* service : _services) {
    service_futures.push_back(service->initialize(_queue.get()));
  }

  // Run the gRPC queue on a background thread so the long-running loop waiting
  // on the completion queue does not block this thread's scheduler.
  log(INFO) << "gRPC services started, executing completion queue.";
  co_await thread([&]() -> co::Task {
    void* tag = nullptr;
    bool ok = false;
    while (_queue->Next(&tag, &ok)) {
      if (!ok) {
        // TODO: Handle this failure mode correctly.
        throw Internal() << "Queue unexpectedly not ok.";
      }
      LW_CHECK_NULL_INTERNAL(tag) << "Queue tag came back null!?";
      static_cast<QueueProcessor*>(tag)->grpc_queue_tick();
    }
    co_return;
  });

  log(INFO) << "gRPC queue exited. Awaiting service shutdowns.";
  co_await co::all_void(service_futures.begin(), service_futures.end());
  log(INFO) << "gRPC services finished, exiting.";
}

}
