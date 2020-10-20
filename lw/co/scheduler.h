#pragma once

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
namespace internal {
class EPoll;
}

class Scheduler;

typedef int Handle;

/**
 * An opaque reference to a task running in a scheduler.
 */
class TaskRef {
public:
  TaskRef(): _task{nullptr} {}
  TaskRef(const TaskRef&) = default;
  TaskRef& operator=(const TaskRef&) = default;
  TaskRef(TaskRef&&) = default;
  TaskRef& operator=(TaskRef&&) = default;
  ~TaskRef() = default;

  operator bool() const { return _task != nullptr; }
  bool operator==(std::nullptr_t) const { return _task == nullptr; }

private:
  TaskRef(Task<void>* task): _task{task} {}

  Task<void>* _task;

  friend class Scheduler;
};

/**
 * A per-thread singleton coroutine scheduling service.
 *
 * To get an instance of this class, use `Scheduler::this_thread` or
 * `Scheduler::for_thread`.
 */
class Scheduler {
public:
  ~Scheduler();

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
   * Fetches a reference to the currently active test.
   *
   * @throw FailedPrecondition
   *  If there is no active task at the time of calling this method.
   *
   * TODO(alaina): Add tests.
   */
  TaskRef current_task() const;

  /**
   * Suspends the calling task, to be resumed on the next tick.
   *
   * Only call this on the `this_thread` scheduler.
   *
   * TODO(alaina): Add thread-checking enforcement.
   */
  auto next_tick() {
    _add_to_queue(_active_task);
    return std::suspend_always{};
  }

  /**
   * Resumes the task on the scheduler's thread.
   *
   * TODO(alaina): Make this thread safe.
   */
  void schedule(Task<void> task) {
    _add_to_queue(new Task<void>(std::move(task)));
  }

  /**
   * Resumes the task on the scheduler's thread.
   *
   * @throw InvalidArgument
   *  If the TaskRef is invalid (task == nullptr).
   *
   * TODO(alaina): Add tests.
   */
  void schedule(TaskRef task);

  /**
   * Schedules the given task for execution. Upon completion, the callback will
   * be called with the result of the task.
   *
   * TODO(alaina): Implement this functionality. :)
   */
  template <typename T, typename Func>
  void schedule(Task<T> task, Func&& callback);

  /**
   * Schedules a resumption of the current task when the given events fire.
   *
   * @param handle
   *  The OS handle/file descriptor the events will trigger on.
   * @param events
   *  The set of events to watch for.
   */
  void schedule(Handle handle, Event events);

  /**
   * Runs the event loop until it is empty.
   */
  void run();

private:
  Scheduler();

  void _add_to_queue(Task<void>* task);
  void _schedule_queue_drain();
  void _schedule(Handle handle, Event events, std::function<void()> func);
  void _resume(Task<void>* task);

  std::unique_ptr<internal::EPoll> _epoll;
  Task<void>* _active_task = nullptr;
  CircularQueue<Task<void>*> _task_queue;
  Handle _event_fd = 0;
};

}
