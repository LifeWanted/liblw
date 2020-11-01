#pragma once

#include <algorithm>
#include <cstdint>
#include <string>
#include <string_view>

#include "lw/co/future.h"
#include "lw/co/scheduler.h"
#include "lw/memory/buffer.h"

namespace lw::io::testing {

/**
 * A CoReadable class backed by a std::string.
 *
 * This class is only useful for testing. If you find a non-testing use for this
 * you should rearchitect your solution.
 */
class StringReadable {
public:
  explicit StringReadable(std::string_view str): _str{str} {}

  bool eof() const { return !good(); }
  bool good() const { return _read_pos < _str.size(); }

  co::Future<std::size_t> read(Buffer& buffer) {
    co_await co::next_tick();
    std::size_t read_size = std::min(_str.size() - _read_pos, buffer.size());
    buffer.copy(_str.begin() + _read_pos, read_size);
    _read_pos += read_size;
    co_return read_size;
  }

private:
  std::size_t _read_pos = 0;
  std::string _str;
};

}
