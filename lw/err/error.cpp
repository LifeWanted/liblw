#include "lw/err/error.h"

#include <cstring>
#include <experimental/source_location>
#include <string_view>

namespace lw {

using ::std::experimental::source_location;

Error::Error(const Error& other):
  std::runtime_error{""},
  _stack{other._stack},
  _stream{&_stack._buffer}
{}

Error::Error(Error&& other):
  std::runtime_error{""},
  _stack{std::move(other._stack)},
  _stream{&_stack._buffer}
{}

Error& Error::operator=(const Error& other) {
  _stack = other._stack;
  return *this;
}

Error& Error::operator=(Error&& other) {
  _stack = std::move(other._stack);
  _stream.rdbuf(&_stack._buffer);
  return *this;
}

Error::Error(std::string_view name, const source_location& loc):
  std::runtime_error{""},
  _stack{loc},
  _stream{&_stack._buffer}
{
  _stream << loc.file_name() << ':' << loc.line() << ": " << name << ": ";
}

ErrorStack::ErrorStack(const source_location& loc): _location{loc} {}

ErrorStack::ErrorStack(const ErrorStack& other):
  _location{other._location},
  _buffer{other._buffer.string()}
{
  if (other._previous) {
    _previous = std::make_unique<ErrorStack>(*other._previous);
  }
}

ErrorStack& ErrorStack::operator=(const ErrorStack& rhs) {
  _location = rhs._location;
  _buffer.reserve(rhs._buffer.size());
  std::memcpy(_buffer.data(), rhs._buffer.data(), _buffer.size());
  if (rhs._previous) _previous = std::make_unique<ErrorStack>(*rhs._previous);
  return *this;
}

void Error::_set_previous(ErrorStack previous) {
  _stack._previous = std::make_unique<ErrorStack>(std::move(previous));
}

}
