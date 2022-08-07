#include <memory>

#include "lw/co/event_system.h"
#include "lw/co/systems/epoll.h"

namespace lw::co {

std::unique_ptr<EventSystem> make_scheduler_event_system() {
  return std::make_unique<internal::EPoll>();
}

}
