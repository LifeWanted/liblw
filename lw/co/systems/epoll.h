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
  void add(int fd, Event events, callback_type callback);

  /**
   * Stops watching for events on the file descriptor and destroys the callback.
   */
  void remove(int fd);

  /**
   * Returns true if there are any pending callbacks.
   */
  bool has_pending_items() const { return !_callbacks.empty(); }

  /**
   * Wait indefinitely for an event to trigger.
   *
   * @return
   *  The number of events that were triggered.
   */
  std::size_t wait() { return _wait(-1); }

  /**
   * Fire any events that are ready and return immediately, even if there are
   * no events ready.
   *
   * @return
   *  The number of events that were triggered.
   */
  std::size_t try_wait() { return _wait(0); }

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
  std::size_t wait_for(std::chrono::steady_clock::duration timeout);

private:
  std::size_t _wait(int timeout_ms);

  int _epoll_fd = -1;
  std::unordered_map<int, std::pair<callback_type, bool>> _callbacks;
};

}
