#pragma once

#include <cstdint>
#include <type_traits>
#include <vector>

#include "lw/err/canonical.h"

namespace lw {

template <typename T>
concept Moveable =
  std::is_move_assignable_v<T> && std::is_move_constructible_v<T>;

/**
 * An oroborous queue. No bytes are shifted when popping, and there is no
 * indirection in the storage.
 *
 * @tparam T
 *  The type to store in the queue. Must be moveable.
 */
template <Moveable T>
class CircularQueue {
public:
  /**
   * Create a queue qith `capacity` slots.
   */
  explicit CircularQueue(std::size_t capacity):
    _capacity{capacity}
  {
    _buffer.reserve(capacity);
  }

  CircularQueue(CircularQueue&& other):
    _capacity{other._capacity},
    _buffer{std::move(other._buffer)},
    _head{other._head},
    _tail{other._tail}
  {
    other._capacity = 0;
    other._head = 0;
    other._tail = 0;
  }

  CircularQueue& operator=(CircularQueue&& other) {
    _capacity = other._capacity;
    _buffer = std::move(other._buffer);
    _head = other._head;
    _tail = other._tail;

    other._capacity = 0;
    other._head = 0;
    other._tail = 0;

    return *this;
  }

  CircularQueue(const CircularQueue&) = delete;
  CircularQueue& operator=(const CircularQueue&) = delete;

  ~CircularQueue() = default;

  std::size_t size() const { return _size; }
  bool empty() const { return size() == 0; }
  bool full() const { return size() == _capacity; }

  T pop_front() {
    if (empty()) {
      throw FailedPrecondition() << "CircularQueue is empty, nothing to pop.";
    }
    std::size_t old_tail = _tail;
    _tail = _next_index(_tail);
    --_size;
    return std::move(_buffer[old_tail]);
  }

  /**
   * Attempts to add `value` to the queue. Returns `true` if successful,
   * otherwise returns `false`.
   */
  bool try_push_back(T value) {
    if (full()) {
      return false;
    }
    std::size_t old_head = _head;
    _head = _next_index(_head);
    _buffer[old_head] = std::move(value);
    ++_size;
    return true;
  }

  void push_back(T value) {
    if (!try_push_back(std::move(value))) {
      throw ResourceExhausted()
        << "CircularQueue at capacity (" << _capacity
        << "), cannot add more items.";
    }
  }

  template <typename... Args>
  void emplace_back(Args&&... args) {
    push_back(T{std::forward<Args>(args)...});
  }

private:
  std::size_t _next_index(std::size_t idx) const {
    return (idx + 1) % _capacity;
  }

  std::size_t _size = 0;
  std::size_t _capacity;
  std::vector<T> _buffer;
  std::size_t _head = 0;
  std::size_t _tail = 0;
};

}
