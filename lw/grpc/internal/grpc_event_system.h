#pragma once

#include <chrono>
#include <thread>

#include "grpcpp/grpcpp.h"
#include "lw/co/event_system.h"
#include "lw/co/events.h"

namespace lw::grpc::internal {

class GrpcEventSystem : public co::EventSystem {
public:
  explicit GrpcEventSystem(::grpc::CompletionQueue& queue);
  ~GrpcEventSystem();
  GrpcEventSystem(GrpcEventSystem&&) = delete;
  GrpcEventSystem& operator=(GrpcEventSystem&&) = delete;
  GrpcEventSystem(const GrpcEventSystem&) = delete;
  GrpcEventSystem& operator=(const GrpcEventSystem&) = delete;

  /**
   * Adds the given file descriptor to be watched by epoll.
   *
   * @param fd
   *  The file descriptor to watch for events on.
   * @param events
   *  The set of events to monitor for.
   * @param callback
   *  The function to call once an event triggers.
   */
  void add(int fd, co::Event events, callback_type callback) override;

  /**
   * Stops watching for events on the file descriptor and destroys the callback.
   */
  void remove(int fd) override;

  /**
   * Returns true if there are any pending callbacks.
   */
  bool has_pending_items() const override { return _pending_items; }

  /**
   * Wait indefinitely for an event to trigger.
   *
   * @return
   *  The number of events that were triggered.
   */
  std::size_t wait() override;

  /**
   * Fire any events that are ready and return immediately, even if there are
   * no events ready.
   *
   * @return
   *  The number of events that were triggered.
   */
  std::size_t try_wait() override {
    return wait_for(std::chrono::milliseconds(0));
  }

  /**
   * Wait for any events to fire or `timeout` to expire, whichever comes first.
   *
   * @throw ::lw::InvalidArgument
   *  If `timeout` is less than 1 or greater than max int.
   *
   * @param timeout
   *  The maximum amount of time to wait. Resolution is no finer than
   *  milliseconds and could be coarser.
   *
   * @return
   *  The number of events that were triggered.
   */
  std::size_t wait_for(std::chrono::steady_clock::duration timeout) override;

private:
  bool _pending_items = true;
  int _epoll_fd = -1;
  ::grpc::CompletionQueue& _queue;
  std::thread _epoll_pump;
};

}
