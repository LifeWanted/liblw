#pragma once

#include <iterator>
#include <ostream>

#include "lw/memory/buffer_view.h"

namespace lw::format {
namespace internal {

template <class Iterator>
class BlobView {
public:
  explicit BlobView(BufferView buffer): _buffer{buffer} {}

  Iterator begin() const { return Iterator{*this}; }
  Iterator end() const { return Iterator{*this, _buffer.size()}; }
  std::size_t size() const { return _buffer.size() * 2; }

private:
  friend Iterator;
  BufferView _buffer;
};

class HexIterator {
public:
  explicit HexIterator(const BlobView<HexIterator>& view): _buffer{view._buffer} {
    _load_chars();
  }

  explicit HexIterator(const BlobView<HexIterator>& view, std::size_t start):
    _buffer{view._buffer},
    _index{start}
  {}

  const char& operator*() const {
    return _hex[_char_index];
  }

  HexIterator& operator++();
  HexIterator& operator--();

  std::int64_t operator-(const HexIterator& other) const {
    return _position() - other._position();
  }

  bool operator==(const HexIterator& other) const;
  bool operator!=(const HexIterator& other) const {
    return !(*this == other);
  }

private:
  void _load_chars();

  std::size_t _position() const {
    return _index * 2 + _char_index;
  }

  BufferView _buffer;
  char _hex[2];
  std::size_t _index = 0;
  std::uint8_t _char_index = 0;
};

template <class Iterator>
std::ostream& operator<<(std::ostream& out, const BlobView<Iterator>& view) {
  for (char c : view) out << c;
  return out;
}

}

// -------------------------------------------------------------------------- //

/**
 * Provides a hex-formatted view of the buffer.
 *
 * Iterating through the returned view will provide one nibble at a time encoded
 * in hex.
 *
 * @return
 *  A container which converts the buffer contents into hexadecimal characters
 *  as it is iterated through. The view can be directly passed to output streams
 *  and supports bidirectional iterator semantics.
 */
inline internal::BlobView<internal::HexIterator> hex(BufferView buffer) {
  return internal::BlobView<internal::HexIterator>{buffer};
}

}

template<>
struct std::iterator_traits<::lw::format::internal::HexIterator> {
  typedef std::int64_t difference_type;
  typedef const char value_type;
  typedef const char* pointer;
  typedef const char& reference;
  typedef std::bidirectional_iterator_tag iterator_category;
};
