#include "lw/co/scheduler.h"

#include <memory>
#include <thread>
#include <unordered_map>

#include "lw/co/systems/epoll.h"
#include "lw/err/canonical.h"

namespace lw::co {
namespace {

using ::std::chrono::steady_clock;

static std::unordered_map<std::thread::id, Scheduler*> thread_schedulers;

}

Scheduler::Scheduler() {
  if (thread_schedulers.contains(std::this_thread::get_id())) {
    throw FailedPrecondition()
      << "Thread " << std::this_thread::get_id() << " already has a scheduler.";
  }
  thread_schedulers.insert({std::this_thread::get_id(), this});
}

Scheduler& Scheduler::this_thread() {
  static thread_local Scheduler instance;
  return instance;
}

Scheduler& Scheduler::for_thread(std::thread::id thread_id) {
  auto itr = thread_schedulers.find(thread_id);
  if (itr == thread_schedulers.end()) {
    throw NotFound() << "Thread " << thread_id << " does not have a scheduler.";
  }
  return *itr->second;
}

void Scheduler::_schedule(delayed_task task) {
  _delayed_tasks.push(std::move(task));

}

}
