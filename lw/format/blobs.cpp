#include "lw/format/blobs.h"

#include <algorithm>
#include <cstring>
#include <ostream>
#include <string_view>

namespace lw::format {
namespace internal {
namespace {

char to_hex_char(int n) {
  return n < 10 ? ('0' + n) : ('a' + n - 10);
}

void to_base_64(char* out, BufferView buffer, std::size_t offset) {
  // TODO(alaina): Create a URL-safe version of this too using - and _.
  static const std::string_view b64_table =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

  std::size_t outpos        = 0;
  int bits_collected        = 0;
  unsigned int accumulator  = 0;

  // Work on every character of the source string...
  for (; outpos < 4 && offset < buffer.size(); ++offset) {
    accumulator = (accumulator << 8) | (buffer[offset] & 0xffu);
    bits_collected += 8;

    // ...and add groups of 6 bits to the resulting string.
    // Remember that 3 real bytes = 4 base64 bytes
    while (bits_collected >= 6) {
      bits_collected -= 6;
      out[outpos++] = b64_table[(accumulator >> bits_collected) & 0x3fu];
    }
  }

  // Sometimes, some bits are left over. Add these to the string at the end.
  if (bits_collected > 0) {
    accumulator <<= 6 - bits_collected;
    out[outpos++] = b64_table[accumulator & 0x3fu];
  }
  while (outpos < 4) out[outpos++] = '=';
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

// -------------------------------------------------------------------------- //

Base64Iterator& Base64Iterator::operator++() {
  if (_char_index == 3) {
    _index = std::min(_index + 3, _buffer.size());
    _char_index = 0;
    _load_chars();
  } else {
    ++_char_index;
  }

  return *this;
}

Base64Iterator& Base64Iterator::operator--() {
  if (_char_index == 0) {
    _index -= 3;
    _char_index = 3;
    _load_chars();
  } else {
    --_char_index;
  }
  return *this;
}

bool Base64Iterator::operator==(const Base64Iterator& other) const {
  return (
    _buffer == other._buffer &&
    _index == other._index &&
    _char_index == other._char_index
  );
}

void Base64Iterator::_load_chars() {
  if (_index > _buffer.size()) {
    std::memset(_block, '\0', 4);
    return;
  }
  to_base_64(_block, _buffer, _index);
}

}
}
