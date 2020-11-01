#pragma once

#include <cstdint>

namespace lw::co {

enum class Event {
  READABLE      = 0b0000000000000001,
  WRITABLE      = 0b0000000000000010,
  READ_CLOSED   = 0b0000000000000100,
  PEER_CLOSED   = 0b0000000000001000, // Implicitly set by epoll.
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

}
