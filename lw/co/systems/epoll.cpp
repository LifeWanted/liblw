#include "lw/co/systems/epoll.h"

#include <cstdint>
#include <exception>
#include <limits>
#include <sys/epoll.h>
#include <unistd.h>

#include "lw/err/canonical.h"
#include "lw/err/system.h"
#include "lw/flags/flags.h"

LW_FLAG(
  std::uint16_t, lw_epoll_event_buffer_size, 32,
  "Size of buffer to create when waiting for events trigger."
);

namespace lw::co::internal {
namespace {

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

}

EPoll::EPoll() {
  _epoll_fd = ::epoll_create1(/*flags=*/0);
  if (_epoll_fd <= 0) {
    check_system_error();
    throw Internal() << "Unknown error from epoll_create1.";
  }
}

EPoll::~EPoll() {
  if (_epoll_fd > 0) ::close(_epoll_fd);
}

void EPoll::add(int fd, Event events, callback_type callback) {
  if (_callbacks.contains(fd)) {
    throw FailedPrecondition() << "Handle already registered with epoll.";
  }

  // Register to event with epoll.
  ::epoll_event ev = {
    .events = co_event_to_epoll_event(events),
    .data = {.fd = fd}
  };
  if (::epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, fd, &ev) != 0) {
    check_system_error();
    throw Internal() << "Unknown error from epoll_ctl when adding handle.";
  }

  // Add the callback to our map.
  _callbacks.insert({fd, {std::move(callback), events & Event::ONE_SHOT}});
}

void EPoll::remove(int fd) {
  if (_callbacks.erase(fd) == 0) {
    throw FailedPrecondition() << "Handle not registered with epoll.";
  }
  if (::epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, fd, nullptr) != 0) {
    check_system_error();
    throw Internal() << "Unknown error from epoll_ctl when removing handle.";
  }
}

std::size_t EPoll::wait_for(std::chrono::steady_clock::duration timeout) {
  auto timeout_ms =
    std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count();
  if (timeout_ms <= 0) {
    throw InvalidArgument() << "Timeout must be a positive duration.";
  } else if (timeout_ms > std::numeric_limits<int>::max()) {
    throw InvalidArgument()
      << "Timeout can be no longer than " << std::numeric_limits<int>::max()
      << " milliseconds.";
  }
  return _wait(static_cast<int>(timeout_ms));
}

std::size_t EPoll::_wait(int timeout_ms) {
  std::vector<::epoll_event> events{
    flags::lw_epoll_event_buffer_size.value(),
    {.events = 0, .data = {.fd = 0}}
  };

  int available_events = ::epoll_wait(
    _epoll_fd,
    events.data(),
    static_cast<int>(events.size()),
    timeout_ms
  );
  if (available_events < 0) {
    check_system_error();
    throw Internal() << "Unknown error from epoll_wait.";
  } else if (static_cast<std::uint32_t>(available_events) > events.size()) {
    throw Internal()
      << "Epoll returned " << available_events << " events in a buffer of size "
      << events.size();
  }

  for (int i = 0; i < available_events; ++i) {
    const ::epoll_event& event = events[i];
    if (!_callbacks.contains(event.data.fd)) {
      throw Internal()
        << "Received event trigger for handle not in callbacks map.";
    }
    auto& [callback, one_shot] = _callbacks[event.data.fd];
    try {
      callback();
      if (one_shot) _callbacks.erase(event.data.fd);
    } catch (const std::exception& err) {
      // TODO: handle this error more gracefully and reject the associated
      // promise higher up.
      throw Internal() << "Error thrown by callback:\n\t" << err.what();
    } catch (...) {
      throw Internal() << "Unknown error thrown by callback!";
    }
  }

  return static_cast<std::size_t>(available_events);
}

}
