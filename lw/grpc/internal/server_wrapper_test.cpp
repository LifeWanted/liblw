#include "lw/grpc/internal/server_wrapper.h"

#include <chrono>
#include <exception>
#include <thread>

#include "gmock/gmock.h"
#include "grpcpp/grpcpp.h"
#include "gtest/gtest.h"
#include "lw/co/future.h"
#include "lw/co/scheduler.h"
#include "lw/co/task.h"
#include "lw/co/testing/destroy_scheduler.h"
#include "lw/co/time.h"
#include "lw/err/canonical.h"
#include "lw/grpc/internal/queue_processor.h"
#include "lw/grpc/internal/service_wrapper.h"
#include "lw/grpc/testing/proto/greeter_service.grpc.pb.h"
#include "lw/log/log.h"
#include "lw/thread/thread.h"

namespace lw::grpc::internal {
namespace {

using ::grpc::ServerCompletionQueue;
using ::lw::co::testing::destroy_all_schedulers;

struct TestQueueProcessor : public QueueProcessor {
  explicit TestQueueProcessor(co::Scheduler& scheduler):
    scheduler{scheduler},
    responder{&context}
  {}
  ~TestQueueProcessor() = default;

  void grpc_queue_tick() override {
    if (state == 1) {
      // testing::HelloRequest req_copy;
      // req_copy.CopyFrom(req);
      // EXPECT_EQ(req.name(), "Alice");
      co::Scheduler& queue_scheduler = co::Scheduler::this_thread();
      scheduler.schedule([&]() -> co::Task {
        testing::HelloResponse res;
        // res.set_message("Hello, " + req_copy.name());
        state = 2;
        queue_scheduler.schedule([&]() -> co::Task {
          responder.Finish(res, /*status=*/{}, (void*)this);
          co_return;
        });

        // The request task will complete once gRPC has indicated it is
        // finished.
        co_await response_promise.get_future();
        request_promise.set_value();
      });
    } else if (state == 2) {
      response_promise.set_value();
    } else {
      throw Internal() << "Unknown state: " << state;
    }
  }

  co::Scheduler& scheduler;
  co::Promise<void> response_promise;
  co::Promise<void> request_promise;
  ::grpc::ServerContext context;
  testing::HelloRequest req;
  ::grpc::ServerAsyncResponseWriter<testing::HelloResponse> responder;
  int state = 1;
};

TEST(GrpcServerWrapper, GreeterService) {
  ServerWrapper server{50051};

  // Add a request runner to the service wrapper.
  testing::GreeterService::AsyncService grpc_service;
  ServiceWrapper service{&grpc_service};
  service.add_runner([&](ServerCompletionQueue* queue) -> co::Future<void> {
    // This request runner handles a single `GreeterService.SayHello` RPC.
    auto processor = std::make_unique<TestQueueProcessor>(
      co::Scheduler::this_thread()
    );
    grpc_service.RequestSayHello(
      &processor->context,
      &processor->req,
      &processor->responder,
      /*new_call_cq=*/queue,
      /*notification_cq=*/queue,
      /*tag=*/processor.get()
    );
    co_await processor->request_promise.get_future();
  });

  // Attach the service and throw the server into the event loop to run.
  server.attach_service(&service);
  co::Scheduler::this_thread().schedule([&]() -> co::Task {
    EXPECT_NO_THROW(co_await server.run());
  });

  // Schedule making a request to the greeter service.
  co::Scheduler::this_thread().schedule([&]() -> co::Task {
    co_await co::sleep_for(std::chrono::milliseconds(500));
    co_await thread([]() -> co::Task {
      auto channel = ::grpc::CreateChannel(
        "localhost:50051",
        ::grpc::InsecureChannelCredentials()
      );
      auto stub = testing::GreeterService::NewStub(channel);
      ::grpc::ClientContext context;
      testing::HelloRequest req;
      testing::HelloResponse res;
      req.set_name("Alice");
      ::grpc::Status status = stub->SayHello(&context, req, &res);
      EXPECT_TRUE(status.ok());
      EXPECT_EQ(res.message(), "Hello, Alice");
      co_return;
    });
    server.close();
  });

  co::Scheduler::this_thread().run();
  destroy_all_schedulers();
}

}
}
