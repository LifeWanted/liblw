#pragma once

#include <atomic>
#include <coroutine>
#include <cstdint>
#include <functional>
#include <memory>
#include <queue>
#include <thread>
#include <vector>

#include "lw/co/concepts.h"
#include "lw/co/events.h"
#include "lw/co/task.h"
#include "lw/memory/circular_queue.h"

namespace lw::co {

class Scheduler;
typedef int Handle;

namespace internal {
class EPoll;
}

/**
 * A per-thread singleton coroutine scheduling service.
 *
 * To get an instance of this class, use `Scheduler::this_thread` or
 * `Scheduler::for_thread`.
 */
class Scheduler {
public:
  Scheduler(Scheduler&&) = delete;
  Scheduler& operator=(Scheduler&&) = delete;
  Scheduler(const Scheduler&) = delete;
  Scheduler& operator=(const Scheduler&) = delete;
  ~Scheduler();

  /**
   * Fetches the Scheduler instance operating on the current thread.
   */
  static Scheduler& this_thread();

  /**
   * Fetches the Scheduler instance operating on the given thread.
   */
  static Scheduler& for_thread(std::thread::id thread_id);

  /**
   * Schedules the given task for execution. Upon completion, the callback will
   * be called with the result of the task.
   */
  template <typename Callback>
  void schedule(Task task, Callback&& callback) {
    task.then(callback);
    _add_to_queue(task.handle());
  }

  /**
   * Invokes the coroutine and schedules the returned coroutine handle for
   * resumption on the next tick of the loop.
   *
   * TODO(alaina): Add type check that Coroutine returns a handle or Task.
   */
  template <CallableCoroutine Coroutine>
  void schedule(Coroutine&& coroutine) { schedule(coroutine()); }

  template <CallableCoroutine Coroutine, typename Func>
  void schedule(Coroutine&& coroutine, Func&& callback) {
    schedule(coroutine(), std::forward<Func>(callback));
  }

  void schedule(std::coroutine_handle<> coro) {
    _add_to_queue(std::move(coro));
  }

  /**
   * Resumes the task on the scheduler's thread.
   *
   * TODO(alaina): Make this thread safe.
   */
  void schedule(Task task) {
    _add_to_queue(task.handle());
  }

  /**
   * Schedules a resumption of the current task when the given events fire.
   *
   * @param coro
   *  The coroutine that will be resumed when the event fires.
   * @param handle
   *  The OS handle/file descriptor the events will trigger on.
   * @param events
   *  The set of events to watch for.
   */
  void schedule(std::coroutine_handle<> coro, Handle handle, Event events);

  /**
   * Runs the event loop until it is empty or stop is called.
   */
  void run();

  /**
   * Signals the event loop to stop. Safe to call from any thread.
   */
  void stop();

private:
  Scheduler();

  void _add_to_queue(std::coroutine_handle<> coro);
  void _schedule_queue_drain();
  void _schedule(Handle handle, Event events, std::function<void()> func);

  std::atomic_bool _continue_polling = true;
  std::unique_ptr<internal::EPoll> _epoll;
  CircularQueue<std::coroutine_handle<>> _coro_queue;
  Handle _queue_notification_fd = 0;
};

// -------------------------------------------------------------------------- //

namespace internal {

struct NextTickAwaitable {
  bool await_ready() const { return false; }
  void await_suspend(std::coroutine_handle<> coro) const {
    Scheduler::this_thread().schedule(std::move(coro));
  }
  void await_resume() const {}
};

struct EventsAwaitable {
  /**
   * Creates an awaitable that will schedule the coroutine to resume when the
   * given event files on the handle.
   *
   * @param handle
   *  The OS handle/file descriptor the events will trigger on.
   * @param events
   *  The set of events to watch for.
   */
  EventsAwaitable(Handle handle, Event events): _handle{handle}, _events{events}
  {}

  bool await_ready() const { return false; }
  void await_suspend(std::coroutine_handle<> coro) const {
    Scheduler::this_thread().schedule(std::move(coro), _handle, _events);
  }
  void await_resume() const {}

private:
  Handle _handle;
  Event _events;
};

}

/**
 * Suspends the coroutine, to be resumed on the next tick.
 */
inline auto next_tick() { return internal::NextTickAwaitable{}; }

/**
 * Schedules a resumption of the current task handle is readable.
 *
 * @param fd
 *  The OS handle/file descriptor the events will trigger on.
 */
inline auto fd_readable(Handle fd) {
  return internal::EventsAwaitable{fd, Event::READABLE | Event::ONE_SHOT};
}

/**
 * Schedules a resumption of the current task handle is writable.
 *
 * @param fd
 *  The OS handle/file descriptor the events will trigger on.
 */
inline auto fd_writable(Handle fd) {
  return internal::EventsAwaitable{fd, Event::WRITABLE | Event::ONE_SHOT};
}

}
