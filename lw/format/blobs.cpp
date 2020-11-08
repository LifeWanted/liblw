#include "lw/format/blobs.h"

#include <ostream>

namespace lw::format {
namespace internal {
namespace {

char to_hex_char(int n) {
  return n < 10 ? ('0' + n) : ('a' + n - 10);
}

}

HexIterator& HexIterator::operator++() {
  if (_char_index == 1) {
    ++_index;
    _char_index = 0;
    _load_chars();
  } else {
    _char_index = 1;
  }
  return *this;
}

HexIterator& HexIterator::operator--() {
  if (_char_index == 0) {
    --_index;
    _char_index = 1;
    _load_chars();
  } else {
    _char_index = 0;
  }
  return *this;
}

bool HexIterator::operator==(const HexIterator& other) const {
  return (
    _buffer == other._buffer &&
    _index == other._index &&
    _char_index == other._char_index
  );
}

void HexIterator::_load_chars() {
  if (_index < _buffer.size()) {
    _hex[0] = to_hex_char((_buffer[_index] >> 4) & 0x0f);
    _hex[1] = to_hex_char(_buffer[_index] & 0x0f);
  } else {
    _hex[0] = '\0';
    _hex[1] = '\0';
  }
}

}
}
