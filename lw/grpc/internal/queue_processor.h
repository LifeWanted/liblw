#pragma once

namespace lw::grpc::internal {

/**
 * Base class
 */
class QueueProcessor {
public:
  QueueProcessor() = default;
  virtual ~QueueProcessor() = default;

  virtual void grpc_queue_tick() = 0;
};

}
