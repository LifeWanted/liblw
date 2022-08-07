#pragma once

#include "grpcpp/grpcpp.h"

namespace lw::grpc::internal {

const std::unordered_set<::grpc::Service*>& get_registered_services();

}
