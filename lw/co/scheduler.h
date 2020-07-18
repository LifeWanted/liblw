#pragma once

#include <concepts>
#include <cstdint>
#include <functional>
#include <queue>
#include <thread>
#include <type_traits>
#include <vector>

namespace lw::co {

template <typename T>
concept BooleanTestable = std::is_convertible_v<T, bool>;

template <typename T>
concept Awaitable = requires(T a) {
  { a.await_ready() } -> BooleanTestable;
  { a.await_suspend() };
  { a.await_resume() };
};

enum class Event {
  READABLE      = 0b0000000000000001,
  WRITABLE      = 0b0000000000000010,
  READ_CLOSED   = 0b0000000000000100,
  PEER_CLOSED   = 0b0000000000001000,
  POLLPRI       = 0b0000000000010000, // See POLLPRI in poll(2) manual.
  ERROR         = 0b0000000000100000,
  EDGE_TRIGGER  = 0b0000000001000000,
  ONE_SHOT      = 0b0000000010000000,
  WAKE_UP       = 0b0000000100000000,
  EXCLUSIVE     = 0b0000001000000000
};

inline Event operator|(Event lhs, Event rhs) {
  return static_cast<Event>(
    static_cast<std::uint16_t>(lhs) | static_cast<std::uint16_t>(rhs)
  );
}

inline Event operator+(Event lhs, Event rhs) {
  return static_cast<Event>(
    static_cast<std::uint16_t>(lhs) | static_cast<std::uint16_t>(rhs)
  );
}

inline Event operator-(Event lhs, Event rhs) {
  return static_cast<Event>(
    static_cast<std::uint16_t>(lhs) & ~static_cast<std::uint16_t>(rhs)
  );
}

inline bool operator&(Event lhs, Event rhs) {
  return static_cast<std::uint16_t>(lhs) & static_cast<std::uint16_t>(rhs);
}

typedef int Handle;

/**
 * A singleton per-thread coroutine scheduling service.
 */
class Scheduler {
public:
  ~Scheduler() = default;

  Scheduler(Scheduler&&) = delete;
  Scheduler& operation=(Scheduler&&) = delete;

  Scheduler(const Scheduler&) = delete;
  Scheduler& operation=(const Scheduler&) = delete;

  static Scheduler& this_thread();
  static Scheduler& for_thread(std::thread::id thread_id);

  template <Awaitable A>
  void schedule(Handle handle, A awaitable, Event events);

private:
  Scheduler();
};

}