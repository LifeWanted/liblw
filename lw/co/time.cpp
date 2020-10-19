#include "lw/co/time.h"

#include <chrono>
#include <sys/timerfd.h>
#include <unistd.h>

#include "lw/co/scheduler.h"
#include "lw/err/canonical.h"
#include "lw/err/system.h"

namespace lw::co {
namespace internal {

Handle create_timerfd(ClockType type, std::chrono::nanoseconds duration) {
  auto clock_id = type == ClockType::STEADY ? CLOCK_MONOTONIC : CLOCK_REALTIME;
  int timer = ::timerfd_create(clock_id, TFD_NONBLOCK);
  if (timer <= 0) {
    check_system_error();
    throw Internal() << "Unknown system error while creating timer.";
  }

  auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
  ::itimerspec timer_spec{
    .it_interval = {0},
    .it_value = {
      .tv_sec = seconds.count(),
      .tv_nsec = (duration - seconds).count()
    }
  };
  int res = ::timerfd_settime(timer, /*flags=*/0, &timer_spec, nullptr);
  if (res != 0) {
    check_system_error();
    throw Internal() << "Unknown system error setting expiration of timer.";
  }
  return Handle{timer};
}

void close_timerfd(Handle timer) {
  ::close(timer);
}

}
}
