#include "lw/memory/buffer.h"

#include <cstring>

#include "lw/err/macros.h"

namespace lw {

void Buffer::_xor(const Buffer& lhs, const Buffer& rhs, Buffer* out) {
  LW_CHECK_NULL(out);

  for (std::size_t i = 0; i < out->size(); ++i) {
    out->_data[i] = lhs._data[i] ^ rhs._data[i];
  }
}

Buffer& Buffer::operator=(Buffer&& other) {
  // If own our current data, delete it first.
  if (_own_data && _data) {
    delete[] _data;
  }

  // Copy the information over.
  _data     = other._data;
  _capacity = other._capacity;
  _own_data = other._own_data;

  // Remove ownership of the buffer from the other one.
  other._own_data = false;

  // Return self.
  return *this;
}

void Buffer::set_memory(std::uint8_t val) {
  std::memset(_data, val, _capacity);
}

bool Buffer::operator==(const Buffer& other) const {
  if (size() != other.size()) {
    return false;
  }
  if (data() == other.data()) {
    return true;
  }

  return std::memcmp(data(), other.data(), size()) == 0;
}

Buffer& Buffer::operator^=(const Buffer& other) {
  // Stupid Windows defines a "min" macro that conflicts with std::min.
  using namespace std;
  Buffer tmp(_data, min(this->size(), other.size()), false);
  _xor(*this, other, &tmp);
  return *this;
}

std::uint8_t& Buffer::front() {
  LW_CHECK_NULL(_data) << "Buffer does not contain any data.";
  return *_data;
}
const std::uint8_t& Buffer::front() const {
  LW_CHECK_NULL(_data) << "Buffer does not contain any data.";
  return *_data;
}

std::uint8_t& Buffer::back() {
  LW_CHECK_NULL(_data) << "Buffer does not contain any data.";
  return *(_capacity ? _data + _capacity - 1 : _data);
}
const std::uint8_t& Buffer::back() const {
  LW_CHECK_NULL(_data) << "Buffer does not contain any data.";
  return *(_capacity ? _data + _capacity - 1 : _data);
}


}
