#include "lw/mime/json.h"

#include "lw/err/canonical.h"

namespace lw::mime {

JSONSerializationFormatter::JSONSerializationFormatter(std::ostream& output):
  _output{output}
{
  _state.push(State::VALUE);
}

void JSONSerializationFormatter::put_null() {
  _add_literal("null");
}

void JSONSerializationFormatter::put_boolean(bool boolean) {
  _add_literal(boolean ? "true" : "false");
}

void JSONSerializationFormatter::put_char(char c) {
  _check_can_add_string();
  _maybe_comma();
  _output << '"';
  _put_char(c);
  _output << '"';
  _string_added();
}

void JSONSerializationFormatter::put_signed_integer(std::int64_t number) {
  _check_can_add_value();
  _maybe_comma();
  _output << number;
  _value_added();
}

void JSONSerializationFormatter::put_unsigned_integer(std::uint64_t number) {
  _check_can_add_value();
  _maybe_comma();
  _output << number;
  _value_added();
}

void JSONSerializationFormatter::put_floating_point(double number) {
  _check_can_add_value();
  _maybe_comma();
  _output << number;
  _value_added();
}

void JSONSerializationFormatter::put_string(std::string_view str) {
  _check_can_add_string();
  _maybe_comma();
  _output << '"';
  for (char c : str) _put_char(c);
  _output << '"';
  _string_added();
}

void JSONSerializationFormatter::start_list() {
  _check_can_add_value();
  _maybe_comma();
  _output << '[';
  _state.push(State::LIST);
}

void JSONSerializationFormatter::end_list() {
  if (_state.top() != State::LIST && _state.top() != State::LIST_VALUE) {
    throw FailedPrecondition()
      << "Unexpected end of list while formatting JSON.";
  }
  _output << ']';
  _value_added();
}

void JSONSerializationFormatter::start_object() {
  _check_can_add_value();
  _maybe_comma();
  _output << '{';
  _state.push(State::OBJECT);

}

void JSONSerializationFormatter::end_object() {
  if (_state.top() != State::OBJECT && _state.top() != State::OBJECT_VALUE) {
    throw FailedPrecondition()
      << "Unexpected end of object while formatting JSON.";
  }
  _output << '}';
  _value_added();
}

void JSONSerializationFormatter::start_pair_key() {
  if (_state.top() != State::OBJECT && _state.top() != State::OBJECT_VALUE) {
    throw FailedPrecondition()
      << "Unexpected start of object key while formatting JSON.";
  }
  _state.push(State::KEY_STARTED);
}

void JSONSerializationFormatter::end_pair_key() {
  if (_state.top() != State::KEY_ADDED) {
    throw FailedPrecondition()
      << "Unexpected end of object key while formatting JSON.";
  }
  _output << ':';
  _state.pop();
  _state.push(State::KEY_ENDED);
}

void JSONSerializationFormatter::end_pair() {
  if (_state.top() != State::OBJECT && _state.top() != State::OBJECT_VALUE) {
    throw FailedPrecondition()
      << "Unexpected end of object value while formatting JSON.";
  }
}

void JSONSerializationFormatter::_maybe_comma() {
  if (
    _state.top() == State::LIST_VALUE ||
    _state.top() == State::OBJECT_VALUE
  ) {
    _output << ',';
  }
}

void JSONSerializationFormatter::_check_can_add_value(
  const std::experimental::source_location& loc
) {
  if (
    _state.top() != State::VALUE &&
    _state.top() != State::LIST &&
    _state.top() != State::LIST_VALUE &&
    _state.top() != State::KEY_ENDED
  ) {
    throw InvalidArgument(loc) << "Unexpected value while formatting JSON.";
  }
}

void JSONSerializationFormatter::_check_can_add_string(
  const std::experimental::source_location& loc
) {
  if (
    _state.top() != State::VALUE &&
    _state.top() != State::LIST &&
    _state.top() != State::LIST_VALUE &&
    _state.top() != State::KEY_STARTED &&
    _state.top() != State::KEY_ENDED
  ) {
    throw InvalidArgument(loc) << "Unexpected string while formatting JSON.";
  }
}

void JSONSerializationFormatter::_value_added() {
  if (_state.top() == State::LIST) {
    _state.pop();
    _state.push(State::LIST_VALUE);
  } else if (_state.top() == State::OBJECT) {
    _state.pop();
    _state.push(State::OBJECT_VALUE);
  } else if (
    _state.top() == State::VALUE ||
    _state.top() == State::KEY_ENDED
  ) {
    _state.pop();
  } else {
    throw Internal() << "Invalid state reached for adding a value.";
  }
}

void JSONSerializationFormatter::_string_added() {
  if (_state.top() == State::KEY_STARTED) {
    _state.pop();
    _state.push(State::KEY_ADDED);
  } else {
    _value_added();
  }
}

void JSONSerializationFormatter::_add_literal(std::string_view value) {
  _check_can_add_value();
  _maybe_comma();
  _output << value;
  _value_added();
}

void JSONSerializationFormatter::_put_char(char c) {
  if (c == '"' || c == '\\' || c == '/') {
    _output << '\\' << c;
  } else if (c == '\b') {
    _output << "\\b";
  } else if (c == '\f') {
    _output << "\\f";
  } else if (c == '\n') {
    _output << "\\n";
  } else if (c == '\r') {
    _output << "\\r";
  } else if (c == '\t') {
    _output << "\\t";
  } else {
    _output << c;
  }
}

}
