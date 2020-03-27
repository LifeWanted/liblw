#pragma once

#include <streambuf>
#include <string>

namespace lw::io::stream {

/**
 * A buffer built on `std::string` that provides copy-free access to the
 * underlying string.
 */
class StringBuffer: public std::streambuf {
public:
  StringBuffer();
  StringBuffer(StringBuffer&&);
  StringBuffer(const StringBuffer&) = delete;
  StringBuffer& operator=(StringBuffer&&);
  StringBuffer& operator=(const StringBuffer&) = delete;

  char_type* data() { return _buffer.data(); }
  const char_type* data() const { return _buffer.data(); }
  const char_type* c_str() const { return _buffer.c_str(); }
  const std::size_t size() const { return _buffer.size(); }
  const std::size_t capacity() const { return _buffer.capacity(); }
  const std::string& string() const { return _buffer; }

  void reserve(std::size_t new_capacity);

protected:
  std::streamsize _get_size() const { return gptr() - _buffer.data(); }
  std::streamsize showmanyc() override { return _get_size(); }
  std::streamsize xsgetn(char_type* out, std::streamsize count) override;
  std::streamsize xsputn(const char_type* in, std::streamsize count) override;

private:
  std::string _buffer;
};

}
