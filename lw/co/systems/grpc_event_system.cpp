#include "lw/co/systems/grpc_event_system.h"

#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <unistd.h>

#include "grpcpp/alarm.h"
#include "grpcpp/grpcpp.h"
#include "lw/err/canonical.h"
#include "lw/err/macros.h"
#include "lw/err/system.h"
#include "lw/log/log.h"

namespace lw::co::internal {
namespace {

struct GrpcEvent {
  GrpcEvent() = default;
  virtual ~GrpcEvent() = default;

  virtual void tick() = 0;
};

class EpollEvent : public GrpcEvent {
public:
  static EpollEvent* from_callback(EventSystem::callback_type callback) {
    return new EpollEvent(std::move(callback));
  }

  void tick() override {
    _callback();
    delete this;
  }

  ::grpc::Alarm& alarm() { return _alarm; }

private:
  explicit EpollEvent(EventSystem::callback_type callback):
    _callback{std::move(callback)}
  {}
  ~EpollEvent() = default;

  EventSystem::callback_type _callback;
  ::grpc::Alarm _alarm;
};

std::uint32_t co_event_to_epoll_event(Event events) {
  std::uint32_t ret = 0;
  if (events & Event::READABLE)     ret |= EPOLLIN;
  if (events & Event::WRITABLE)     ret |= EPOLLOUT;
  if (events & Event::READ_CLOSED)  ret |= EPOLLRDHUP;
  if (events & Event::PEER_CLOSED)  ret |= EPOLLHUP;
  if (events & Event::POLLPRI)      ret |= EPOLLPRI;
  if (events & Event::ERROR)        ret |= EPOLLERR;
  if (events & Event::EDGE_TRIGGER) ret |= EPOLLET;
  if (events & Event::ONE_SHOT)     ret |= EPOLLONESHOT;
  if (events & Event::WAKE_UP)      ret |= EPOLLWAKEUP;
  if (events & Event::EXCLUSIVE)    ret |= EPOLLEXCLUSIVE;
  return ret;
}

void run_epoll(int epoll_fd, ::grpc::CompletionQueue& queue) {
  epoll_event event;
  std::chrono::system_clock::time_point immediately;
  while (::epoll_wait(epoll_fd, &event, /*maxevents=*/1, /*timeout=*/-1) == 1) {
    if (event.data.ptr == nullptr) break;
    EpollEvent* e = static_cast<EpollEvent*>(event.data.ptr);
    e->alarm().Set(&queue, /*deadline=*/immediately, e);
  }

  // Likely epoll_wait returned 0 indicating it was closed, but check for errors
  // just in case.
  check_system_error();
  log(INFO) << "gRPC Epoll thread shut down cleanly.";
}

int signal_epoll_stop(int epoll) {
  int evfd = ::eventfd(/*initval=*/0, EFD_NONBLOCK);
  ::epoll_event ev = {.events = EPOLLIN, .data = {.ptr = nullptr}};
  if (::epoll_ctl(epoll, EPOLL_CTL_ADD, evfd, &ev) != 0) {
    check_system_error();
    throw Internal() << "Unknown error signalling epoll closure.";
  }
  std::int64_t val = 1;
  if (::write(evfd, &val, sizeof(val)) < static_cast<int>(sizeof(val))) {
    check_system_error();
    throw Internal() << "Unknown error writing epoll shutdown signal.";
  }
  return evfd;
}

}

GrpcEventSystem::GrpcEventSystem(::grpc::CompletionQueue& queue):
  _queue{queue}
{
  _epoll_fd = ::epoll_create1(/*flags=*/0);
  if (_epoll_fd <= 0) {
    check_system_error();
    throw Internal() << "Unknown error from epoll_create1.";
  }
  _epoll_pump = std::thread{[&]() { run_epoll(_epoll_fd, _queue); }};
}

GrpcEventSystem::~GrpcEventSystem() {
  int shutdown_signal = -1;
  if (_epoll_fd > 0 && _epoll_pump.joinable()) {
    shutdown_signal = signal_epoll_stop(_epoll_fd);
  }
  if (_epoll_fd > 0) ::close(_epoll_fd);
  if (_epoll_pump.joinable()) _epoll_pump.join();
  if (shutdown_signal > 0) ::close(shutdown_signal);
}

void GrpcEventSystem::add(int fd, Event events, callback_type callback) {
  ::epoll_event ev = {
    .events = co_event_to_epoll_event(events),
    .data = {.ptr = EpollEvent::from_callback(std::move(callback))}
  };
  if (::epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, fd, &ev) != 0) {
    check_system_error();
    throw Internal() << "Unknown error from epoll_ctl when adding handle.";
  }
}

void GrpcEventSystem::remove(int fd) {
  // TODO(alainawolfe) Any EpollEvent objects in this will be leaked at this
  // point.
  if (::epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, fd, nullptr) != 0) {
    int err_code = get_system_error();
    if (err_code == EBADFD) {
      throw InvalidArgument() << "File descriptor is invalid.";
    }
    check_system_error(err_code);
    throw Internal() << "Unknown error from epoll_ctl when removing handle.";
  }
}

std::size_t GrpcEventSystem::wait() {
  void* tag = nullptr;
  bool ok = false;
  if (!_queue.Next(&tag, &ok)) return 0;
  if (!ok || !tag) throw Internal() << "Error waiting on gRPC queue!";

  static_cast<GrpcEvent*>(tag)->tick();
  return 1;
}

std::size_t GrpcEventSystem::wait_for(
  std::chrono::steady_clock::duration timeout
) {
  LW_CHECK_GTE_TYPED(timeout.count(), 0, InvalidArgument)
    << "Timeout must not be negative.";

  void* tag = nullptr;
  bool ok = false;
  std::chrono::system_clock::time_point deadline;
  if (timeout.count() > 0) {
    deadline = std::chrono::system_clock::now() + timeout;
  }
  ::grpc::CompletionQueue::NextStatus status =
    _queue.AsyncNext(&tag, &ok, deadline);
  if (status != ::grpc::CompletionQueue::GOT_EVENT) return 0;
  if (!ok || !tag) throw Internal() << "Error waiting on gRPC queue!";

  static_cast<GrpcEvent*>(tag)->tick();
  return 1;
}

}
