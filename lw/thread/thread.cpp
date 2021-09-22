#include "lw/thread/thread.h"

#include <coroutine>
#include <cstdint>
#include <sys/eventfd.h>
#include <unistd.h>

#include "lw/co/events.h"
#include "lw/co/scheduler.h"
#include "lw/err/canonical.h"
#include "lw/err/system.h"

namespace lw {
namespace internal {
namespace {

co::Handle create_eventfd() {
  co::Handle fd = ::eventfd(/*initval=*/0, EFD_NONBLOCK);
  if (fd <= 0) {
    check_system_error();
    throw Internal() << "Unknown error from creating eventfd.";
  }
  return fd;
}

void close_eventfd(co::Handle fd) {
  std::int64_t val = 0;
  if (::read(fd, &val, sizeof(val)) < static_cast<int>(sizeof(val))) {
    check_system_error();
    throw Internal() << "Unknown error reading from eventfd.";
  }
  if (::close(fd) != 0) {
    check_system_error();
    throw Internal() << "Unknown error closing eventfd.";
  }
}

void ping_eventfd(co::Handle fd) {
  std::int64_t val = 1;
  if (::write(fd, &val, sizeof(val)) < static_cast<std::int64_t>(sizeof(val))) {
    check_system_error();
    throw Internal() << "Unknown error writing to eventfd.";
  }
}

}

ThreadAwaitable::State::State(): event_handle{create_eventfd()} {}

ThreadAwaitable::State::~State() {
  if (thread.joinable()) thread.join();
  close_eventfd(event_handle);
}

void ThreadAwaitable::State::complete() {
  finished = true;
  ping_eventfd(event_handle);
}

void ThreadAwaitable::await_suspend(std::coroutine_handle<> coro) {
  co::Scheduler::this_thread().schedule(
    coro,
    _state->event_handle,
    co::Event::READABLE | co::Event::ONE_SHOT
  );
}

}
}
