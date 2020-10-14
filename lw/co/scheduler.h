#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <queue>
#include <thread>
#include <vector>

#include "lw/co/concepts.h"
#include "lw/co/events.h"

namespace lw::co {
namespace internal {
class EPoll;
}

typedef int Handle;

/**
 * A per-thread singleton coroutine scheduling service.
 *
 * To get an instance of this class, use `Scheduler::this_thread` or
 * `Scheduler::for_thread`.
 */
class Scheduler {
public:
  ~Scheduler() = default;

  Scheduler(Scheduler&&) = delete;
  Scheduler& operator=(Scheduler&&) = delete;
  Scheduler(const Scheduler&) = delete;
  Scheduler& operator=(const Scheduler&) = delete;

  /**
   * Fetches the Scheduler instance operating on the current thread.
   */
  static Scheduler& this_thread();

  /**
   * Fetches the Scheduler instance operating on the given thread.
   */
  static Scheduler& for_thread(std::thread::id thread_id);

  /**
   * Adds an awaitable which will be resumed when any of the given events
   * trigger on the handle.
   *
   * @param handle
   *  The OS handle/file descriptor the events will trigger on.
   * @param events
   *  The set of events to watch for.
   * @param awaitable
   *  The awaitable to resume once an event triggers. This object will be
   *  forwarded and moved in the process of scheduling it.
   */
  template <Awaitable Awaiter>
  void schedule(Handle handle, Event events, Awaiter&& awaitable) {
    _schedule(
      handle,
      events,
      [awaitable{std::forward<Awaiter>(awaitable)}]() mutable {
        awaitable.await_resume();
      }
    );
  }

  /**
   * Runs the event loop until it is empty.
   */
  void run();

private:
  Scheduler();

  void _schedule(Handle handle, Event events, std::function<void()> func);

  std::unique_ptr<internal::EPoll> _epoll;
};

}
