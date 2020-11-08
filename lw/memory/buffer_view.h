#pragma once

#include <cstdint>
#include <string_view>

#include "lw/memory/buffer.h"

namespace lw {

class BufferView {
public:
  BufferView() = default;
  BufferView(BufferView&&) = default;
  BufferView(const BufferView& other): BufferView{other._buffer} {}

  BufferView& operator=(BufferView&&) = default;
  BufferView& operator=(const BufferView& other) {
    _buffer = Buffer{
      const_cast<std::uint8_t*>(other._buffer.data()),
      other._buffer.size(),
      /*own_data=*/false
    };
    return *this;
  }

  BufferView(const Buffer& buffer):
    _buffer{
      const_cast<std::uint8_t*>(buffer.data()),
      buffer.size(),
      /*own_data=*/false
    }
  {}

  BufferView(const std::uint8_t* data, std::size_t size):
    _buffer{const_cast<std::uint8_t*>(data), size, /*own_data=*/false}
  {}

  std::size_t capacity() const { return _buffer.capacity(); }
  std::size_t size() const { return _buffer.size(); }
  [[nodiscard]] bool empty() const { return _buffer.empty(); }
  const std::uint8_t* data() const { return _buffer.data(); }
  const std::uint8_t& operator[](std::size_t i) const { return _buffer[i]; }
  const std::uint8_t* begin() const { return _buffer.begin(); }
  const std::uint8_t* end() const { return _buffer.end(); }
  const std::uint8_t& front() const { return _buffer.front(); }
  const std::uint8_t& back() const { return _buffer.back(); }

  /**
   * Two BufferViews are equal if they point to the same address and are of the
   * same size.
   */
  bool operator==(const BufferView& other) const {
    return (
      _buffer.data() == other._buffer.data() &&
      _buffer.size() == other._buffer.size()
    );
  }

  /**
   * Convert a BufferView to a string_view.
   */
  operator std::string_view() const {
    return std::string_view{reinterpret_cast<const char*>(begin()), size()};
  }

private:
  Buffer _buffer;
};

}
