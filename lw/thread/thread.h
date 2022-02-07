#pragma once

#include <atomic>
#include <coroutine>
#include <functional>
#include <memory>
#include <thread>

#include "lw/co/concepts.h"
#include "lw/co/future.h"
#include "lw/co/scheduler.h"

namespace lw {
namespace internal {

class ThreadAwaitable {
public:
  struct State {
    State();
    ~State();

    void complete();
    std::thread thread;
    std::atomic_bool finished = false;
    co::Handle event_handle;
  };

  explicit ThreadAwaitable(std::shared_ptr<State> state): _state{state} {}

  bool await_ready() const { return _state->finished; }
  void await_suspend(std::coroutine_handle<> coro);
  void await_resume() const {}

private:
  std::shared_ptr<State> _state;
};

}

/**
 * Spawns a new thread to schedule the provided coroutine.
 *
 * @param coroutine
 *  The coroutine to run on a new thread.
 */
template <co::CallableCoroutine Coroutine>
auto thread(Coroutine&& coroutine) {
  auto state = std::make_shared<internal::ThreadAwaitable::State>();
  state->thread = std::thread{
    [state, coroutine = std::forward<Coroutine>(coroutine)]() {
      co::Scheduler::this_thread().schedule(coroutine);
      co::Scheduler::this_thread().run();
      state->complete();
    }
  };
  return internal::ThreadAwaitable{std::move(state)};
}

/**
 * @brief Spawns a new thread and calls the given function on it.
 *
 * @param func
 *  The function to call from the new thread.
 *
 * @return An awaitable that resolves when the thread finishes.
 */
internal::ThreadAwaitable thread(void (*func)());
internal::ThreadAwaitable thread(std::function<void()> func);

}
