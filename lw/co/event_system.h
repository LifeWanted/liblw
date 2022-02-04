#pragma once

#include <chrono>
#include <functional>

#include "lw/co/events.h"

namespace lw::co {

/**
 * Interface for event systems that the co::Scheduler may build upon.
 */
class EventSystem {
public:
  typedef std::function<void()> callback_type;

  EventSystem() = default;
  virtual ~EventSystem() = default;
  EventSystem(EventSystem&&) = default;
  EventSystem& operator=(EventSystem&&) = default;
  EventSystem(const EventSystem&) = delete;
  EventSystem& operator=(const EventSystem&) = delete;

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
  virtual void add(int fd, Event events, callback_type callback) = 0;

  /**
   * Stops watching for events on the file descriptor and destroys the callback.
   */
  virtual void remove(int fd) = 0;

  /**
   * Returns true if there are any pending callbacks.
   */
  virtual bool has_pending_items() const = 0;

  /**
   * Wait indefinitely for an event to trigger.
   *
   * @return
   *  The number of events that were triggered.
   */
  virtual std::size_t wait() = 0;

  /**
   * Fire any events that are ready and return immediately, even if there are
   * no events ready.
   *
   * @return
   *  The number of events that were triggered.
   */
  virtual std::size_t try_wait() = 0;

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
  virtual std::size_t wait_for(std::chrono::steady_clock::duration timeout) = 0;
};

}
