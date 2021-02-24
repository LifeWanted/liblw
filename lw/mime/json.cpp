#include "lw/mime/json.h"

#include <cctype>
#include <charconv>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <ostream>
#include <stack>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

#include "lw/err/canonical.h"
#include "lw/io/serializer/parser.h"

namespace lw::mime {

// -------------------------------------------------------------------------- //
//                                                                            //
//          #####  ###  ####  #     #  ###  ##### ##### ##### ####            //
//          #     #   # #   # ##   ## #   #   #     #   #     #   #           //
//          ####  #   # ####  # # # # #####   #     #   ####  ####            //
//          #     #   # #   # #  #  # #   #   #     #   #     #   #           //
//          #      ###  #   # #     # #   #   #     #   ##### #   #           //
//                                                                            //
// -------------------------------------------------------------------------- //

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
  _maybe_comma();
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
  _value_added();
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
  switch (_state.top()) {
    case State::LIST: {
      _state.pop();
      _state.push(State::LIST_VALUE);
      break;
    }
    case State::OBJECT: {
      _state.pop();
      _state.push(State::OBJECT_VALUE);
      break;
    }
    case State::VALUE:
    case State::KEY_ENDED: {
      _state.pop();
      break;
    }
    case State::LIST_VALUE:
    case State::OBJECT_VALUE: {
      // Noop.
      break;
    }
    default: {
      throw Internal() << "Invalid state reached for adding a value.";
    }
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

// -------------------------------------------------------------------------- //
//                                                                            //
//                  ####   ###  ####   ###  ##### ####                        //
//                  #   # #   # #   # #     #     #   #                       //
//                  ####  ##### ####   ###  ####  ####                        //
//                  #     #   # #   #     # #     #   #                       //
//                  #     #   # #   #  ###  ##### #   #                       //
//                                                                            //
// -------------------------------------------------------------------------- //

namespace {

enum class JSONType: int {
  NUL = 0,
  BOOLEAN,
  STRING,
  INTEGER,
  FLOAT,
  LIST,
  OBJECT
};

std::string_view json_type_name(JSONType type) {
  static std::string_view names[] = {
    "NUL",
    "BOOLEAN",
    "STRING",
    "INTEGER",
    "FLOAT",
    "LIST",
    "OBJECT"
  };
  return names[static_cast<int>(type)];
}

/**
 * Parses and decodes a JSON-encoded string, including quotation marks, into a
 * plain C++ string.
 *
 * @todo UTF-8 decoding.
 */
std::string decode_string(std::string_view token) {
  // TODO(#12): Parse "\uXXXX" sequence as a UTF-8 character.
  std::string out;
  out.reserve(token.size());
  for (std::size_t i = 1; i < token.size() - 1; ++i) {
    char c = token.at(i);
    if (c == '\\') {
      switch (token.at(++i)) {
        case '"':
        case '\\':
        case '/': {
          out.push_back(token.at(i));
          break;
        }
        case 'b': {
          out.push_back('\b');
          break;
        }
        case 'f': {
          out.push_back('\f');
          break;
        }
        case 'n': {
          out.push_back('\n');
          break;
        }
        case 'r': {
          out.push_back('\r');
          break;
        }
        case 't': {
          out.push_back('\t');
          break;
        }
      }
    } else {
      out.push_back(c);
    }
  }
  return out;
}

class JSONScalarToken: public io::DeserializationToken {
public:
  JSONScalarToken(JSONType type, std::string_view token):
    _type{type},
    _token{token}
  {
    if (_type == JSONType::STRING) _decoded_string = decode_string(token);
  }
  ~JSONScalarToken() = default;

  JSONType type() const {
    return _type;
  }
  std::string_view token() const {
    return _token;
  }

  std::size_t size() const override {
    if (is_string()) return _decoded_string.size();
    throw FailedPrecondition()
      << "Token is " << json_type_name(_type) << " which does not have a size.";
  }

  bool is_null() const override { return _type == JSONType::NUL; }
  bool is_boolean() const override { return _type == JSONType::BOOLEAN; }
  bool is_char() const override { return is_string() && size() == 1; }
  bool is_signed_integer() const override { return _type == JSONType::INTEGER; }
  bool is_unsigned_integer() const override {
    return _type == JSONType::INTEGER && _token.front() != '-';
  }
  bool is_floating_point() const override {
    return _type == JSONType::INTEGER || _type == JSONType::FLOAT;
  }
  bool is_string() const override { return _type == JSONType::STRING; }
  bool is_list() const override { return false; }
  bool is_object() const override { return false; }

  bool get_boolean() const override {
    if (!is_boolean()) {
      throw FailedPrecondition()
        << "Token is " << json_type_name(_type) << ", not "
        << json_type_name(JSONType::BOOLEAN);
    }
    return _token == "true";
  }

  char get_char() const override {
    if (!is_char()) {
      throw FailedPrecondition()
        << "Token is " << json_type_name(_type) << ", not CHAR";
    }
    return _decoded_string.front();
  }

  std::int64_t get_signed_integer() const override {
    if (!is_signed_integer()) {
      throw FailedPrecondition()
        << "Token is " << json_type_name(_type) << ", not "
        << json_type_name(JSONType::INTEGER);
    }

    auto begin = _token.begin();
    if (*begin == '+') ++begin;
    std::int64_t out;
    auto [end, err] = std::from_chars(begin, _token.end(), out);
    if (end != _token.end() || err == std::errc::invalid_argument) {
      throw InvalidArgument()
        << "String " << _token << " is not a valid signed integer.";
    } else if (err == std::errc::result_out_of_range) {
      throw InvalidArgument()
        << "Value " << _token << " is too large for a 64-bit integer.";
    }
    return out;
  }

  std::uint64_t get_unsigned_integer() const override {
    if (!is_unsigned_integer()) {
      throw FailedPrecondition()
        << "Token is " << json_type_name(_type) << ", not unsigned "
        << json_type_name(JSONType::INTEGER);
    }

    auto begin = _token.begin();
    if (*begin == '+') ++begin;
    std::uint64_t out;
    auto [end, err] = std::from_chars(begin, _token.end(), out);
    if (end != _token.end() || err == std::errc::invalid_argument) {
      throw InvalidArgument()
        << "String " << _token << " is not a valid unsigned integer.";
    } else if (err == std::errc::result_out_of_range) {
      throw InvalidArgument()
        << "Value " << _token
        << " is too large for an unsigned 64-bit integer.";
    }
    return out;
  }

  double get_floating_point() const override {
    if (!is_floating_point()) {
      throw FailedPrecondition()
        << "Token is " << json_type_name(_type) << ", not "
        << json_type_name(JSONType::FLOAT);
    }

    char* end = nullptr;
    double out = std::strtod(_token.begin(), &end);
    if (end != _token.end()) {
      throw InvalidArgument()
        << "String \"" << _token << "\" is not a valid double value.";
    } else if (out == HUGE_VAL) {
      throw InvalidArgument()
        << "Value \"" << _token << "\" is too large for double.";
    }
    return out;
  }

  std::string_view get_string() const override {
    if (!is_string()) {
      throw FailedPrecondition()
        << "Token is " << json_type_name(_type) << ", not "
        << json_type_name(JSONType::STRING);
    }
    return _decoded_string;
  }

  bool has_index(std::size_t idx) const override { return false; }
  const io::DeserializationToken& get_index(std::size_t idx) const override {
    throw FailedPrecondition()
      << "Token is " << json_type_name(_type) << ", not "
      << json_type_name(JSONType::LIST);
  }

  bool has_key(std::string_view key) const override { return false; }
  const io::DeserializationToken& get_key(std::string_view key) const override {
    throw FailedPrecondition()
      << "Token is " << json_type_name(_type) << ", not "
      << json_type_name(JSONType::LIST);
  }

private:
  JSONType _type;
  std::string_view _token;
  std::string _decoded_string;
};

class JSONListToken: public io::DeserializationToken {
public:
  JSONListToken() = default;
  ~JSONListToken() = default;

  void push_back(std::unique_ptr<io::DeserializationToken> value) {
    _elements.push_back(std::move(value));
  }

  std::size_t size() const override { return _elements.size(); }

  bool is_null() const override { return false; }
  bool is_boolean() const override { return false; }
  bool is_char() const override { return false; }
  bool is_signed_integer() const override { return false; }
  bool is_unsigned_integer() const override { return false; }
  bool is_floating_point() const override { return false; }
  bool is_string() const override { return false; }
  bool is_list() const override { return true; }
  bool is_object() const override { return false; }

  bool get_boolean() const override {
    throw FailedPrecondition()
      << "Token is " << json_type_name(JSONType::LIST) << ", not "
      << json_type_name(JSONType::BOOLEAN);
  }
  char get_char() const override {
    throw FailedPrecondition()
      << "Token is " << json_type_name(JSONType::LIST) << ", not "
      << json_type_name(JSONType::STRING);
  }
  std::int64_t get_signed_integer() const override {
    throw FailedPrecondition()
      << "Token is " << json_type_name(JSONType::LIST) << ", not "
      << json_type_name(JSONType::INTEGER);
  }
  std::uint64_t get_unsigned_integer() const override {
    throw FailedPrecondition()
      << "Token is " << json_type_name(JSONType::LIST) << ", not "
      << json_type_name(JSONType::INTEGER);
  }
  double get_floating_point() const override {
    throw FailedPrecondition()
      << "Token is " << json_type_name(JSONType::LIST) << ", not "
      << json_type_name(JSONType::FLOAT);
  }
  std::string_view get_string() const override {
    throw FailedPrecondition()
      << "Token is " << json_type_name(JSONType::LIST) << ", not "
      << json_type_name(JSONType::STRING);
  }

  bool has_index(std::size_t idx) const override {
    return idx < _elements.size();
  }
  const io::DeserializationToken& get_index(std::size_t idx) const override {
    if (!has_index(idx)) {
      throw OutOfRange()
        << "Index " << idx << " is larger than this list of "
        << _elements.size();
    }
    return *_elements.at(idx);
  }

  bool has_key(std::string_view key) const override { return false; }
  const io::DeserializationToken& get_key(std::string_view key) const override {
    throw FailedPrecondition()
      << "Token is " << json_type_name(JSONType::LIST) << ", not "
      << json_type_name(JSONType::OBJECT);
  }

private:
  std::vector<std::unique_ptr<io::DeserializationToken>> _elements;
};

class JSONObjectToken: public io::DeserializationToken {
public:
  JSONObjectToken() = default;
  ~JSONObjectToken() = default;

  void insert(
    std::string_view key,
    std::unique_ptr<io::DeserializationToken> value
  ) {
    if (has_key(key)) {
      throw FailedPrecondition()
        << "Key " << key << " already added to object.";
    }
    _elements.insert({key, std::move(value)});
  }

  std::size_t size() const override { return _elements.size(); }

  bool is_null() const override { return false; }
  bool is_boolean() const override { return false; }
  bool is_char() const override { return false; }
  bool is_signed_integer() const override { return false; }
  bool is_unsigned_integer() const override { return false; }
  bool is_floating_point() const override { return false; }
  bool is_string() const override { return false; }
  bool is_list() const override { return false; }
  bool is_object() const override { return true; }

  bool get_boolean() const override {
    throw FailedPrecondition()
      << "Token is " << json_type_name(JSONType::OBJECT) << ", not "
      << json_type_name(JSONType::BOOLEAN);
  }
  char get_char() const override {
    throw FailedPrecondition()
      << "Token is " << json_type_name(JSONType::OBJECT) << ", not "
      << json_type_name(JSONType::STRING);
  }
  std::int64_t get_signed_integer() const override {
    throw FailedPrecondition()
      << "Token is " << json_type_name(JSONType::OBJECT) << ", not "
      << json_type_name(JSONType::INTEGER);
  }
  std::uint64_t get_unsigned_integer() const override {
    throw FailedPrecondition()
      << "Token is " << json_type_name(JSONType::OBJECT) << ", not "
      << json_type_name(JSONType::INTEGER);
  }
  double get_floating_point() const override {
    throw FailedPrecondition()
      << "Token is " << json_type_name(JSONType::OBJECT) << ", not "
      << json_type_name(JSONType::FLOAT);
  }
  std::string_view get_string() const override {
    throw FailedPrecondition()
      << "Token is " << json_type_name(JSONType::OBJECT) << ", not "
      << json_type_name(JSONType::STRING);
  }

  bool has_index(std::size_t idx) const override { return false; }
  const io::DeserializationToken& get_index(std::size_t idx) const override {
    throw FailedPrecondition()
      << "Token is " << json_type_name(JSONType::OBJECT) << ", not "
      << json_type_name(JSONType::LIST);
  }

  bool has_key(std::string_view key) const override {
    return _elements.contains(key);
  }
  const io::DeserializationToken& get_key(std::string_view key) const override {
    if (!has_key(key)) {
      throw OutOfRange() << "Key " << key << " is not present in this object.";
    }
    return *_elements.at(key);
  }

private:
  std::unordered_map<
    std::string_view,
    std::unique_ptr<io::DeserializationToken>
  > _elements;
};

typedef std::variant<
  char,
  std::unique_ptr<JSONScalarToken>,
  std::unique_ptr<JSONListToken>,
  std::unique_ptr<JSONObjectToken>
> JSONToken;

bool is_char(const JSONToken& token) {
  return std::holds_alternative<char>(token);
}

bool is_scalar(const JSONToken& token) {
  return std::holds_alternative<std::unique_ptr<JSONScalarToken>>(token);
}

bool is_list(const JSONToken& token) {
  return std::holds_alternative<std::unique_ptr<JSONListToken>>(token);
}

bool is_object(const JSONToken& token) {
  return std::holds_alternative<std::unique_ptr<JSONObjectToken>>(token);
}

const char& as_char(const JSONToken& token) {
  return std::get<char>(token);
}

const std::unique_ptr<JSONScalarToken>& as_scalar(const JSONToken& token) {
  return std::get<std::unique_ptr<JSONScalarToken>>(token);
}

std::unique_ptr<JSONListToken>& as_list(JSONToken& token) {
  return std::get<std::unique_ptr<JSONListToken>>(token);
}

std::unique_ptr<JSONObjectToken>& as_object(JSONToken& token) {
  return std::get<std::unique_ptr<JSONObjectToken>>(token);
}

std::unique_ptr<io::DeserializationToken> to_io_token(JSONToken token) {
  if (is_list(token)) return std::move(as_list(token));
  if (is_object(token)) return std::move(as_object(token));
  return std::move(std::get<std::unique_ptr<JSONScalarToken>>(token));
}

std::string_view token_type_name(const JSONToken& token) {
  if (is_char(token)) {
    return std::string_view{&as_char(token), 1};
  } else if (is_list(token)) {
    return json_type_name(JSONType::LIST);
  } else if (is_object(token)) {
    return json_type_name(JSONType::OBJECT);
  } else {
    return json_type_name(as_scalar(token)->type());
  }
}

class JSONParse {
public:
  JSONParse() = default;
  ~JSONParse() = default;

  std::optional<JSONToken> update(std::string_view str) {
    for (std::size_t i = 0; i < str.size(); ++i) {
      const char c = str[i];
      if (std::isspace(c)) {
        // TODO(alaina): Test the space case(s).
        continue;
      }

      if (_final_value) {
        // TODO(alaina): Test failure modes.
        throw InvalidArgument()
          << "Unexpected additional input after value at character " << i
          << " in JSON.";
      }

      switch (c) {
        case '{': {
          _token_stack.push(std::make_unique<JSONObjectToken>());
          break;
        }
        case '}': {
          if (!is_object(_token_stack.top())) {
            throw InvalidArgument()
              << "Unexpected '}' at character " << i << " in JSON.";
          }
          _pop_top(i);
          break;
        }
        case '[': {
          _token_stack.push(std::make_unique<JSONListToken>());
          break;
        }
        case ']': {
          if (!is_list(_token_stack.top())) {
            throw InvalidArgument()
              << "Unexpected ']' at character " << i << " in JSON.";
          }
          _pop_top(i);
          break;
        }
        case ':': {
          if (_token_stack.empty() || !is_scalar(_token_stack.top())) {
            throw InvalidArgument()
              << "Unexpected ':' at character " << i << " in JSON.";
          }
          _token_stack.push(':');
          break;
        }
        case ',': {
          if (!is_list(_token_stack.top()) && !is_object(_token_stack.top())) {
            throw InvalidArgument()
              << "Unexpected ',' at character " << i << " in JSON.";
          }
          break;
        }
        case '"': {
          i = _parse_string(str, i);
          _pop_top(i);
          break;
        }
        case 't':
        case 'f':
        case 'n': {
          i = _parse_keyword(str, i);
          _pop_top(i);
          break;
        }
        case '+':
        case '-':
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9': {
          i = _parse_number(str, i);
          _pop_top(i);
          break;
        }

        default: {
          throw InvalidArgument()
            << "Unexpected '" << c << "' at character " << i << " in JSON.";
        }
      }
    }

    if (!_token_stack.empty()) {
      throw InvalidArgument() << "Unexpected end of input to JSON parser.";
    }

    return std::move(_final_value);
  }

private:
  std::size_t _parse_string(std::string_view str, std::size_t pos) {
    std::size_t start = pos;
    std::size_t end = pos;
    std::size_t i;
    for (i = pos + 1; i < str.size(); ++i) {
      if (str[i] == '\\') continue;
      if (str[i] == '"') {
        end = i;
        break;
      }
    }
    if (end < i) {
      throw InvalidArgument()
        << "Unexpected end of input at character " << i
        << " in JSON string starting at character " << start;
    }
    _token_stack.push(
      std::make_unique<JSONScalarToken>(
        JSONType::STRING,
        str.substr(start, end - start + 1)
      )
    );
    return end;
  }

  std::size_t _parse_keyword(std::string_view str, std::size_t pos) {
    std::string_view substr = str.substr(pos);
    std::unique_ptr<JSONScalarToken> token;
    if (substr.starts_with("true")) {
      token = std::make_unique<JSONScalarToken>(
        JSONType::BOOLEAN,
        substr.substr(0, 4)
      );
      substr = substr.substr(4);
    } else if (substr.starts_with("false")) {
      token = std::make_unique<JSONScalarToken>(
        JSONType::BOOLEAN,
        substr.substr(0, 5)
      );
      substr = substr.substr(5);
    } else if (substr.starts_with("null")) {
      token = std::make_unique<JSONScalarToken>(
        JSONType::NUL,
        substr.substr(0, 4)
      );
      substr = substr.substr(4);
    }
    if (!token || std::isalnum(substr[0]) || substr[0] == '_') {
      throw InvalidArgument()
        << "Unknown keyword at character " << pos << " in JSON.";
    }
    _token_stack.push(std::move(token));
    return substr.begin() - str.begin() - 1;
  }

  std::size_t _parse_number(std::string_view str, std::size_t pos) {
    std::size_t start = pos;
    JSONType type = JSONType::INTEGER;
    if (str[pos] == '+' || str[pos] == '-') {
      ++pos;
      if (pos >= str.size()) {
        throw InvalidArgument() << "Unexpected end of input to JSON parser.";
      }
    }
    if (str[pos] == '0') {
      ++pos;
      if (pos < str.size() && std::isdigit(str[pos])) {
        throw InvalidArgument()
          << "Unexpected digit at character " << pos
          << " in JSON number. Expected '.' or end of number.";
      }
    } else if (!std::isdigit(str[pos])) {
      throw InvalidArgument()
        << "Unexpected '" << str[pos] << "' at character " << pos
        << " in JSON number.";
    }

    for (; pos < str.size(); ++pos) {
      if (str[pos] == '.') {
        if (type == JSONType::FLOAT) {
          throw InvalidArgument()
            << "Unexpected '.' at character " << pos << " in JSON number.";
        }
        type = JSONType::FLOAT;
      } else if (!std::isdigit(str[pos])) {
        break;
      }
    }
    if (str[pos - 1] == '.') {
      throw InvalidArgument()
        << "Unexpected end of number at character " << pos << " in JSON.";
    }

    _token_stack.push(
      std::make_unique<JSONScalarToken>(type, str.substr(start, pos - start))
    );
    return pos - 1;
  }

  void _pop_top(std::size_t i) {
    JSONToken rhs = std::move(_token_stack.top());
    _token_stack.pop();
    if (_token_stack.empty()) {
      _final_value = std::move(rhs);
      return;
    }
    if (is_char(rhs)) {
      throw InvalidArgument()
        << "Unexpected '" << as_char(rhs) << "'" << " at character " << i
        << " in JSON.";
    }
    if (is_object(_token_stack.top())) {
      if (!is_scalar(rhs) || !as_scalar(rhs)->is_string()) {
        throw InvalidArgument()
          << "Unexpected " << token_type_name(rhs) << " at character " << i
          << " in JSON map. Expected STRING key.";
      }
      _token_stack.push(std::move(rhs));
      return;
    }
    if (is_list(_token_stack.top())) {
      as_list(_token_stack.top())->push_back(to_io_token(std::move(rhs)));
      return;
    }
    if (is_char(_token_stack.top())) {
      const char c = as_char(_token_stack.top());
      if (c == ':') {
        _token_stack.pop();
        JSONToken lhs = std::move(_token_stack.top());
        _token_stack.pop();
        if (!is_object(_token_stack.top())) {
          throw InvalidArgument()
            << "Unexpected key-value pair found at character " << i
            << "outside of JSON object.";
        }
        std::string_view key = as_scalar(lhs)->token();
        key.remove_prefix(1);
        key.remove_suffix(1);
        as_object(_token_stack.top())->insert(key, to_io_token(std::move(rhs)));
        return;
      }
    }

    throw Unimplemented() << "_pop_top not fully implemented yet.";
  }

  std::stack<JSONToken> _token_stack;
  std::optional<JSONToken> _final_value;
};

}

std::unique_ptr<io::DeserializationToken> JSONDeserializationParser::parse(
  std::string_view str
) const {
  JSONParse parser;
  auto token = parser.update(str);
  if (!token) {
    throw InvalidArgument() << "Unexpected end of input to JSON parser.";
  }
  if (is_char(*token)) {
    throw InvalidArgument() << "Malformed JSON document.";
  }
  return to_io_token(std::move(*token));
}

}
