#pragma once

#include <algorithm>
#include <cstdint>
#include <string_view>

namespace lw {

/**
 * A buffer that can store dynamically or statically allocated memory.
 *
 * The buffers can be built around existing memory blocks (optionally taking
 * ownership of them) or can allocate their own blocks.
 *
 * There are no protections in place for reading or writing outside the bounds
 * of the buffer.
 */
class Buffer {
public:
  Buffer() = default;
  Buffer(const Buffer&) = delete;
  Buffer& operator=(const Buffer&) = delete;

  /**
   * Create a buffer wrapped around externally allocated memory, optionally
   * taking ownership of the memory.
   *
   * @param buffer
   *  A pointer to the beginning of the buffer.
   * @param capacity
   *  The size of the buffer in bytes.
   * @param own_data
   *  Flag indicating if this `Buffer` should take ownership of the memory.
   */
  Buffer(std::uint8_t* buffer, std::size_t capacity, bool own_data = false):
    _capacity{capacity},
    _data{buffer},
    _own_data{own_data}
  {}

  /**
   * Move ownership from other to this.
   *
   * The ownership of the data is transfered to this buffer, but the `other`
   * buffer remains pointing to the same block with the same capacity so it
   * can continue being used. You are not guaranteed that the data in `other`
   * will remain valid as this buffer may free the memory if it is destroyed.
   */
  Buffer(Buffer&& other):
    _capacity{other._capacity},
    _data{other._data},
    _own_data{other._own_data}
  {
    other._own_data = false;
  }

  /**
   * Create a buffer which allocates its own memory.
   *
   * @param size The number of bytes to allocate.
   */
  explicit Buffer(std::size_t size):
    _capacity{size},
    _data{new std::uint8_t[size]},
    _own_data{true}
  {}

  /**
   * Generic data-copying constructor.
   *
   * The input iterator must support differences
   * (`operator-(const InputIterator&, const InputIterator&)`) as a way of
   * determining the distance between two iterators. On top of that it must
   * support dereferencing (`operator*()`) and prefix increment
   * (`operator++()`).
   *
   * @tparam InputIterator
   *  A generic forward-iterator whose dereferenced value must be convertible
   *  to `byte`.
   *
   * @param begin
   *  The iterator to begin the copy from.
   * @param end
   *  An iterator just past the end of the data.
   */
  template <typename InputIterator>
  Buffer(InputIterator begin, const InputIterator& end):
    Buffer{(std::size_t)(end - begin)}
  {
    std::copy(begin, end, _data);
  }

  ~Buffer() {
    if (_own_data && _data) {
      delete[] _data;
    }
  }

  std::size_t capacity() const { return _capacity; }
  std::size_t size() const { return capacity(); }
  [[nodiscard]] bool empty() const { return _capacity == 0; }

  std::uint8_t* data() { return _data; }
  const std::uint8_t* data() const { return _data; }

  std::uint8_t& operator[](std::size_t i) { return _data[i]; }
  const std::uint8_t& operator[](std::size_t i) const { return _data[i]; }

  std::uint8_t* begin() { return _data; }
  const std::uint8_t* begin() const { return _data; }

  std::uint8_t* end() { return _data + _capacity; }
  const std::uint8_t* end() const { return _data + _capacity; }

  std::uint8_t& front();
  const std::uint8_t& front() const;
  std::uint8_t& back();
  const std::uint8_t& back() const;

  /**
   * Returns a new buffer starting `n` bytes after this one and with `n` bytes
   * lower capacity.
   */
  Buffer trim_prefix(std::size_t n) const;

  /**
   * Returns a new buffer starting at the same byte as this one but with `n`
   * bytes lower capacity.
   */
  Buffer trim_suffix(std::size_t n) const;

  /**
   * Sets all the bytes in the buffer to `val`.
   *
   * @param val
   *  The byte value to set for all bytes in the buffer.
   */
  void set_memory(std::uint8_t val);

  /**
   * Copies at most `size()` bytes of data.
   *
   * @param begin
   *  The iterator to start from.
   * @param end
   *  The iterator one past the end.
   */
  template <typename InputIterator>
  void copy(InputIterator begin, const InputIterator& end) {
    for (std::size_t i = 0; i < size() && begin != end; ++i, ++begin) {
      _data[i] = *begin;
    }
  }

  /**
   * Copies `std::min(size(), count)` bytes of data.
   *
   * @param begin
   *  The iterator to start from.
   * @param count
   *  The number of bytes to copy.
   */
  template <typename InputIterator>
  void copy(InputIterator begin, const std::size_t count) {
    std::copy_n(begin, std::min(count, size()), _data);
  }

  /**
   * Moves control of a buffer over to this one.
   *
   * The ownership of the data is transfered to this buffer, but the `other`
   * buffer remains pointing to the same block with the same capacity so it
   * can continue being used. You are not guaranteed that the data in `other`
   * will remain valid as this buffer may free the memory if it is destroyed.
   */
  Buffer& operator=(Buffer&& other);

  /**
   * Two buffers are equal if they are the same size and contain exactly the
   * same data, byte for byte.
   */
  bool operator==(const Buffer& other) const;
  bool operator!=(const Buffer& other) const {
      return !(*this == other);
  }

  /**
   * Combines two memory buffers using XOR.
   *
   * Only `std::min( this->size(), other.size() )` bytes are combined.
   */
  Buffer& operator^=(const Buffer& other);

  /**
   * Convert a Buffer to a string_view.
   */
  operator std::string_view() const {
    return std::string_view{reinterpret_cast<const char*>(begin()), size()};
  }

private:
  /**
   * Performs XOR between two buffers into a third.
   */
  static void _xor(const Buffer& lhs, const Buffer& rhs, Buffer* out);

  std::size_t _capacity = 0;
  std::uint8_t* _data = nullptr;
  bool _own_data = false;
};

}
