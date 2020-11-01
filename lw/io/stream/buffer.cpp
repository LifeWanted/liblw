#include "lw/io/stream/buffer.h"

#include <algorithm>
#include <cstring>
#include <streambuf>
#include <string>

namespace lw::io::stream {

StringBuffer::StringBuffer() {
  setg(data(), data(), data());
  setp(data(), data() + capacity());
}

StringBuffer::StringBuffer(const std::string& str): _buffer{str} {
  setg(data(), data(), data());
  setp(data(), data() + capacity());
}

StringBuffer::StringBuffer(StringBuffer&& other):
  _buffer{std::move(other._buffer)}
{
  setg(other.eback(), other.gptr(), other.egptr());
  setp(other.pptr(), other.epptr());
}

StringBuffer& StringBuffer::operator=(StringBuffer&& other) {
  _buffer = std::move(other._buffer);
  setg(other.eback(), other.gptr(), other.egptr());
  setp(other.pptr(), other.epptr());
  return *this;
}

void StringBuffer::reserve(std::size_t new_capacity) {
  const char_type* old_start = data();
  _buffer.reserve(new_capacity);
  setg(
    data(),                         // eback
    data() + (gptr() - old_start),  // gptr
    data() + size() - 1             // egptr
  );
  setp(
    data() + size(),    // pbase, pptr
    data() + capacity() // epptr
  );
}

std::streamsize StringBuffer::xsgetn(char_type* out, std::streamsize count) {
  std::streamsize copied = std::min(count, _get_size());
  std::memcpy(out, _buffer.data(), copied);
  setg(
    data(),             // eback
    gptr() + copied,    // gptr
    data() + size() - 1 // egptr
  );
  return copied;
}

std::streamsize StringBuffer::xsputn(
  const char_type* in,
  std::streamsize count
) {
  if (count + size() > capacity()) {
    reserve((count + size()) * 2);
  }
  _buffer.append(in, count);
  setg(
    data(),             // eback
    gptr(),             // gptr
    data() + size() - 1 // egptr
  );
  setp(
    data() + size(),    // pbase, pptr
    data() + capacity() // epptr
  );
  return count;
}

}
