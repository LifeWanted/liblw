#include "lw/co/scheduler.h"

#include <memory>
#include <thread>
#include <unordered_map>

#include "lw/co/systems/epoll.h"
#include "lw/err/canonical.h"

namespace lw::co {
namespace {

using ::std::chrono::steady_clock;

static std::unordered_map<
  std::thread::id,
  std::unique_ptr<Scheduler>
> thread_schedulers;

}
namespace testing {

void destroy_scheduler(std::thread::id thread_id) {
  thread_schedulers.erase(thread_id);
}

}

Scheduler::Scheduler(): _epoll{std::make_unique<internal::EPoll>()} {
  if (thread_schedulers.contains(std::this_thread::get_id())) {
    throw FailedPrecondition()
      << "Thread " << std::this_thread::get_id() << " already has a scheduler.";
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

void Scheduler::run() {
  auto itr = thread_schedulers.find(std::this_thread::get_id());
  if (itr == thread_schedulers.end() || itr->second.get() != this) {
    throw FailedPrecondition()
      << "Cannot call Scheduler::run from a thread other than the one that "
         "created it.";
  }

  while (_epoll->has_pending_items()) _epoll->wait();
}

void Scheduler::_schedule(
  Handle handle,
  Event events,
  std::function<void()> func
) {
  _epoll->add(handle, events, std::move(func));
}

}
