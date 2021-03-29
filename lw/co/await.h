#include "lw/co/scheduler.h"
#include "lw/co/task.h"

namespace lw::co {
namespace internal {

struct TaskAwaitable {
  explicit TaskAwaitable(Task task): _task{std::move(task)} {}

  bool await_ready() const { return _task.done(); }
  void await_suspend(std::coroutine_handle<> coro) {
    Scheduler::this_thread().schedule(std::move(_task), [coro]() {
      Scheduler::this_thread().schedule(coro);
    });
  }

  void await_resume() const {}

private:
  Task _task;
};

}

inline auto task_completion(Task task) {
  return internal::TaskAwaitable{std::move(task)};
}

}
