#pragma once

#include "grpcpp/grpcpp.h"
#include "lw/err/canonical.h"

namespace lw::grpc {

::grpc::Status to_grpc_status(const CanonicalError& err);

}
