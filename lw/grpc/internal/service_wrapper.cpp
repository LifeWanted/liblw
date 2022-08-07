#include "lw/grpc/internal/service_wrapper.h"

#include <vector>

#include "grpcpp/grpcpp.h"
#include "lw/co/future.h"
#include "lw/err/macros.h"
#include "lw/log/log.h"

namespace lw::grpc::internal {

co::Future<void> ServiceWrapper::initialize(
  ::grpc::ServerCompletionQueue* queue
) {
  LW_CHECK_NULL_INTERNAL(queue);

  log(INFO) << "Starting " << _runners.size() << " gRPC service runners.";
  std::vector<co::Future<void>> futures;
  futures.reserve(_runners.size());
  for (const RequestRunner& runner : _runners) futures.push_back(runner(queue));
  co_await co::all_void(futures.begin(), futures.end());
}

}
