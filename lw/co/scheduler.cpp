#include "lw/co/scheduler.h"

#include <coroutine>
#include <memory>
#include <sys/eventfd.h>
#include <thread>
#include <unistd.h>
#include <unordered_map>

#include "lw/co/events.h"
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
  _coro_queue{flags::lw_scheduler_queue_size.value()}
{
  if (thread_schedulers.contains(std::this_thread::get_id())) {
    throw FailedPrecondition()
      << "Thread " << std::this_thread::get_id() << " already has a scheduler.";
  }
}

Scheduler::~Scheduler() {
  if (_queue_notification_fd) {
    ::close(_queue_notification_fd);
    _queue_notification_fd = 0;
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

void Scheduler::schedule(
  std::coroutine_handle<> coro,
  Handle handle,
  Event events
) {
  _schedule(handle, events, [coro]() { coro.resume(); });
}

void Scheduler::run() {
  _continue_polling = true;
  auto itr = thread_schedulers.find(std::this_thread::get_id());
  if (itr == thread_schedulers.end() || itr->second.get() != this) {
    throw FailedPrecondition()
      << "Cannot call Scheduler::run from a thread other than the one that "
         "created it.";
  }

  while (_continue_polling && _epoll->has_pending_items()) _epoll->wait();
}

void Scheduler::stop() {
  _continue_polling = false;
  _schedule_queue_drain();
}

void Scheduler::_add_to_queue(std::coroutine_handle<> coro) {
  _coro_queue.push_back(std::move(coro));
  _schedule_queue_drain();
}

void Scheduler::_schedule_queue_drain() {
  if (!_queue_notification_fd) _queue_notification_fd = create_eventfd();
  try {
    const auto events = Event::READABLE | Event::ONE_SHOT;
    _schedule(_queue_notification_fd, events, [this]() {
      // Limit ourselves to resuming only the tasks in the queue at the start of
      // this cycle to prevent starvation of epoll.
      clear_eventfd(_queue_notification_fd);
      std::size_t limit = _coro_queue.size();
      for (std::size_t i = 0; i < limit && !_coro_queue.empty(); ++i) {
        _coro_queue.pop_front().resume();
      }
    });
  } catch (const AlreadyExists&) {
    // Ignore this case. Wakeup is already scheduled.
  }
  ping_eventfd(_queue_notification_fd);
}

void Scheduler::_schedule(
  Handle handle,
  Event events,
  std::function<void()> func
) {
  _epoll->add(handle, events, std::move(func));
}

}
