#include "lw/grpc/internal/server_wrapper.h"

#include <exception>
#include <thread>

#include "gmock/gmock.h"
#include "grpcpp/grpcpp.h"
#include "gtest/gtest.h"
#include "lw/co/future.h"
#include "lw/co/scheduler.h"
#include "lw/co/task.h"
#include "lw/co/testing/destroy_scheduler.h"
#include "lw/err/canonical.h"
#include "lw/grpc/internal/service_wrapper.h"
#include "lw/grpc/testing/proto/greeter_service.grpc.pb.h"
#include "lw/log/log.h"
#include "lw/thread/thread.h"

namespace lw::grpc::internal {
namespace {

using ::grpc::ServerCompletionQueue;
using ::lw::co::testing::destroy_all_schedulers;

TEST(GrpcServerWrapper, GreeterService) {
  ServerWrapper server{50051};

  testing::GreeterService::AsyncService grpc_service;
  ServiceWrapper service{&grpc_service};
  service.add_runner([&](ServerCompletionQueue* queue) -> co::Future<void> {
    ::grpc::ServerContext context;
    testing::HelloRequest req;
    ::grpc::ServerAsyncResponseWriter<testing::HelloResponse> responder{
      &context
    };
    grpc_service.RequestSayHello(
      &context,
      &req,
      &responder,
      /*new_call_cq=*/queue,
      /*notification_cq=*/queue,
      /*tag=*/(void*)1
    );

    co::Scheduler& scheduler = co::Scheduler::this_thread();
    co_await thread([&]() -> co::Task {
      void* tag;
      bool ok;
      co::Promise<void> response_promise;
      while (queue->Next(&tag, &ok)) {
        if (!ok) {
          throw Internal() << "Queue unexpectedly not ok.";
        }

        if (tag == (void*)1) {
          scheduler.schedule([&]() -> co::Task {
            EXPECT_EQ(req.name(), "Alice");
            testing::HelloResponse res;
            res.set_message("Hello, " + req.name());
            responder.Finish(res, /*status=*/{}, (void*)2);

            co_await response_promise.get_future();
          });
        } else if (tag == (void*)2) {
          response_promise.set_value();
        } else {
          throw Internal() << "Unknown tag: " << tag;
        }
      }
      co_return;
    });
  });

  server.attach_service(&service);
  co::Scheduler::this_thread().schedule([&]() -> co::Task {
    co_await server.run();
  });

  co::Scheduler::this_thread().schedule([&]() -> co::Task {
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
