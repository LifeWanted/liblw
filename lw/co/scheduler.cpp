#include "lw/co/scheduler.h"

#include <memory>
#include <sys/eventfd.h>
#include <thread>
#include <unistd.h>
#include <unordered_map>

#include "lw/co/systems/epoll.h"
#include "lw/err/canonical.h"
#include "lw/err/macros.h"
#include "lw/err/system.h"
#include "lw/flags/flags.h"

LW_FLAG(
  std::size_t, lw_scheduler_queue_size, 100,
  "Maximum size of the task queue in schedulers."
);

namespace lw::co {
namespace {

using ::std::chrono::steady_clock;

static std::unordered_map<
  std::thread::id,
  std::unique_ptr<Scheduler>
> thread_schedulers;

Handle create_eventfd() {
  Handle fd = ::eventfd(/*initval=*/0, EFD_NONBLOCK);
  if (fd <= 0) {
    check_system_error();
    throw Internal() << "Unknown error from creating eventfd.";
  }
  return fd;
}

void clear_eventfd(Handle fd) {
  std::int64_t val = 0;
  if (::read(fd, &val, sizeof(val)) < static_cast<int>(sizeof(val))) {
    check_system_error();
    throw Internal() << "Unknown error reading from eventfd.";
  }
}

void ping_eventfd(Handle fd) {
  std::int64_t val = 1;
  if (::write(fd, &val, sizeof(val)) < static_cast<int>(sizeof(val))) {
    check_system_error();
    throw Internal() << "Unknown error writing to eventfd.";
  }
}

}

namespace testing {

void destroy_all_schedulers() {
  thread_schedulers.clear();
}

void destroy_scheduler(std::thread::id thread_id) {
  thread_schedulers.erase(thread_id);
}

}

Scheduler::Scheduler():
  _epoll{std::make_unique<internal::EPoll>()},
  _task_queue{flags::lw_scheduler_queue_size.value()}
{
  if (thread_schedulers.contains(std::this_thread::get_id())) {
    throw FailedPrecondition()
      << "Thread " << std::this_thread::get_id() << " already has a scheduler.";
  }
}

Scheduler::~Scheduler() {
  if (_event_fd) {
    ::close(_event_fd);
    _event_fd = 0;
  }
}

Scheduler& Scheduler::this_thread() {
  auto itr = thread_schedulers.find(std::this_thread::get_id());
  if (itr == thread_schedulers.end()) {
    auto [new_itr, existed] = thread_schedulers.insert({
      std::this_thread::get_id(),
      std::unique_ptr<Scheduler>{new Scheduler()}
    });
    itr = new_itr;
  }

  return *itr->second;
}

Scheduler& Scheduler::for_thread(std::thread::id thread_id) {
  auto itr = thread_schedulers.find(thread_id);
  if (itr == thread_schedulers.end()) {
    throw NotFound() << "Thread " << thread_id << " does not have a scheduler.";
  }
  return *itr->second;
}

void Scheduler::schedule(Handle handle, Event events) {
  if (_active_task == nullptr) {
    throw FailedPrecondition()
      << "Cannot schedule event resumption outside the context of a task.";
  }
  _schedule(
    handle,
    events,
    [this, task{_active_task}]() { _resume(task); }
  );
}

void Scheduler::run() {
  auto itr = thread_schedulers.find(std::this_thread::get_id());
  if (itr == thread_schedulers.end() || itr->second.get() != this) {
    throw FailedPrecondition()
      << "Cannot call Scheduler::run from a thread other than the one that "
         "created it.";
  }

  while (_epoll->has_pending_items()) _epoll->wait();
}

void Scheduler::_add_to_queue(Task<void>* task) {
  _task_queue.push_back(task);
  _schedule_queue_drain();
}

void Scheduler::_schedule_queue_drain() {
  if (!_event_fd) _event_fd = create_eventfd();
  try {
    _schedule(_event_fd, Event::READABLE | Event::ONE_SHOT, [this]() {
      // Limit ourselves to resuming only the tasks in the queue at the start of
      // this cycle to prevent starvation of epoll.
      clear_eventfd(_event_fd);
      std::size_t limit = _task_queue.size();
      for (std::size_t i = 0; i < limit && !_task_queue.empty(); ++i) {
        _resume(_task_queue.pop_front());
      }
    });
  } catch (const AlreadyExists&) {
    // Ignore this case. Wakeup is already scheduled.
  }
  ping_eventfd(_event_fd);
}

void Scheduler::_schedule(
  Handle handle,
  Event events,
  std::function<void()> func
) {
  _epoll->add(handle, events, std::move(func));
}

void Scheduler::_resume(Task<void>* task) {
  LW_CHECK_NULL(task);
  Task<void>* prev_task = _active_task;
  _active_task = task;
  task->resume();
  if (task->done()) {
    std::unique_ptr<Task<void>> task_ptr{task}; // This will delete for us.
    task = nullptr;
    task_ptr->get(); // To clear any exceptions.
  }
  _active_task = prev_task;
}

}
