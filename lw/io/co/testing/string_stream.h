#pragma once

#include <algorithm>
#include <cstdint>
#include <string>
#include <string_view>

#include "lw/co/future.h"
#include "lw/co/scheduler.h"
#include "lw/io/co/co.h"
#include "lw/memory/buffer.h"

namespace lw::io::testing {

/**
 * A duplex CoStream class backed by a std::string.
 *
 * This class is only useful for testing. If you find a non-testing use for this
 * you should rearchitect your solution.
 */
class CoStringStream: public CoStream {
public:
  explicit CoStringStream(
    std::string_view in,
    std::string& out
  ):
    _read_str{in},
    _write_str{&out}
  {}

  void close() override { _read_str.clear(); }
  bool eof() const override { return _read_pos >= _read_str.size(); }
  bool good() const override { return !eof() && !_read_str.empty(); }

  co::Future<std::size_t> read(Buffer& buffer) override {
    co_await co::next_tick();
    std::size_t read_size = std::min(_read_str.size(), buffer.size());
    buffer.copy(_read_str.begin() + _read_pos, read_size);
    _read_pos += read_size;
    co_return read_size;
  }

  co::Future<std::size_t> write(const Buffer& buffer) override {
    co_await co::next_tick();
    (*_write_str) += static_cast<std::string_view>(buffer);
    co_return buffer.size();
  }

  std::string_view data() const { return *_write_str; }

private:
  std::size_t _read_pos = 0;
  std::string _read_str;
  std::string* _write_str;
};

}
