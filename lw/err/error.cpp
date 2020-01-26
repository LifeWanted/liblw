#include "lw/err/error.h"

#include <experimental/source_location>
#include <string_view>

namespace lw {

Error::Error(Error&& other):
  std::runtime_error{""},
  _location{std::move(other._location)},
  _buffer{std::move(other._buffer)},
  _stream{&_buffer}
{}

Error& Error::operator=(Error&& other) {
  std::runtime_error{""},
  _location = std::move(other._location);
  _buffer = std::move(other._buffer);
  _stream.rdbuf(&_buffer);
  return *this;
}

Error::Error(
  std::string_view name,
  const std::experimental::source_location& loc
):
  std::runtime_error{""},
  _location{loc},
  _buffer{},
  _stream{&_buffer}
{
  _stream << loc.file_name() << ':' << loc.line() << ": " << name << ": ";
}

}
