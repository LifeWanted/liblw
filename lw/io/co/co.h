#pragma once

#include <algorithm>
#include <cstdint>
#include <string_view>

#include "lw/co/future.h"
#include "lw/err/canonical.h"
#include "lw/flags/flags.h"
#include "lw/io/co/concepts.h"
#include "lw/memory/buffer.h"

LW_DECLARE_FLAG(std::size_t, initial_read_buffer_size);
LW_DECLARE_FLAG(std::size_t, read_block_size);

namespace lw::io {
namespace internal {

void adjust_buffers(
  Buffer& buffer,
  Buffer& read,
  Buffer& write,
  std::size_t desired_write_size
);

}

class BaseCoReader {
public:
  virtual bool eof() const = 0;
  virtual bool good() const = 0;

  virtual co::Future<Buffer> read(std::size_t bytes) = 0;
  virtual co::Future<Buffer> read_until(std::uint8_t c, std::size_t limit = 0) = 0;
  virtual co::Future<Buffer> read_until(
    std::string_view str,
    std::size_t limit = 0
  ) = 0;
};

template <CoReadable Source>
class CoReader: public BaseCoReader {
public:
  explicit CoReader(Source& source):
    _source{source},
    _buffer{flags::initial_read_buffer_size}
  {}

  bool eof() const override { return _read_window.empty() && _source.eof(); }
  bool good() const override  {
    return !_read_window.empty() || _source.good();
  }

  co::Future<Buffer> read(std::size_t bytes) override {
    if (_read_window.size() < bytes) {
      co_await _load_buffer(bytes - _read_window.size());
    }
    Buffer result{_read_window.data(), std::min(bytes, _read_window.size())};
    _read_window = _read_window.trim_prefix(result.size());
    co_return result;
  }

  co::Future<Buffer> read_until(
    std::uint8_t c,
    std::size_t limit = 0
  ) override {
    if (limit == 0) limit = flags::read_block_size;
    for (std::size_t i = 0; i < limit; ++i) {
      if (i >= _read_window.size()) {
        if (!good()) co_return Buffer{};
        co_await _load_buffer(limit - i);
      }
      if (_read_window[i] == c) {
        Buffer result{_read_window.data(), i + 1};
        _read_window = _read_window.trim_prefix(i + 1);
        co_return result;
      }
    }
  }

  co::Future<Buffer> read_until(
    std::string_view str,
    std::size_t limit = 0
  ) override {
    if (limit == 0) limit = flags::read_block_size;
    std::size_t str_pos = 0;
    for (std::size_t i = 0; i < limit; ++i) {
      if (i >= _read_window.size()) {
        if (!good()) co_return Buffer{};
        co_await _load_buffer(limit - i);
      }
      if (_read_window[i] == static_cast<std::uint8_t>(str[str_pos])) {
        if (++str_pos == str.size()) {
          Buffer result{_read_window.data(), i + 1};
          _read_window = _read_window.trim_prefix(i + 1);
          co_return result;
        }
      } else {
        str_pos = 0;
      }
    }
  }

private:
  co::Future<void> _load_buffer(std::size_t limit) {
    // Make sure we have space for the desired data.
    if (limit > _write_window.size()) {
      internal::adjust_buffers(_buffer, _read_window, _write_window, limit);
    }

    // Load up the write_window with the data.
    Buffer buff{_write_window.data(), limit};
    std::size_t bytes_read = co_await _source.read(buff);

    // Adjust our windows to account for the newly added data.
    _write_window = _write_window.trim_prefix(bytes_read);
    _read_window = Buffer{
      _read_window.data(),
      _read_window.size() + bytes_read
    };
  }

  Source& _source;
  Buffer _buffer;
  Buffer _read_window;
  Buffer _write_window;
};

}
