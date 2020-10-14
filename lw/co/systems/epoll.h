#pragma once

#include <chrono>
#include <functional>
#include <sys/epoll.h>
#include <unordered_map>

#include "lw/co/events.h"

namespace lw::co::internal {

class EPoll {
public:
  typedef std::function<void()> callback_type;

  EPoll();
  ~EPoll();
  EPoll(EPoll&&) = delete;
  EPoll& operator=(EPoll&&) = delete;
  EPoll(const EPoll&) = delete;
  EPoll& operator=(const EPoll&) = delete;

  void add(int fd, Event events, callback_type callback);
  void remove(int fd);
  std::size_t wait() { return _wait(-1); }
  std::size_t try_wait() { return _wait(0); }
  std::size_t wait_for(std::chrono::steady_clock::duration timeout);

private:
  std::size_t _wait(int timeout_ms);

  int _epoll_fd = -1;
  std::unordered_map<int, std::pair<callback_type, bool>> _callbacks;
};

}
