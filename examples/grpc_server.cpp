/**
 *
 */

#include "examples/proto/greeter_service.grpc.pb.h"
#include "lw/base/base.h"
#include "lw/flags/flags.h"
#include "lw/grpc/server.h"
#include "lw/grpc/handler.h"
#include "lw/log/log.h"
#include "lw/net/server.h"

LW_FLAG(
  unsigned short, port, 50051,
  "Port for the server to listen on for gRPC requests."
);

namespace {
class SayHelloHandler:
  public lw::grpc::Handler<examples::HelloRequest, examples::HelloResponse>
{
  lw::co::Future<void> run() override {
    response().set_message("Hello " + request().name());
    co_return;
  }
};

LW_REGISTER_GRPC_HANDLER(examples::GreeterService, SayHello, SayHelloHandler);
}

int main(int argc, const char** argv) {
  try {
    if (!lw::init(&argc, argv)) return 0;
    lw::grpc::Server server{{.port = lw::flags::port}};
    server.attach_services();
    lw::log(lw::INFO) << "Server is listening on " << lw::flags::port;
    server.run();
  }
}
