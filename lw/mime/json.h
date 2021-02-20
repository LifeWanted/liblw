#pragma once

#include <experimental/source_location>
#include <memory>
#include <stack>
#include <string_view>
#include <ostream>

#include "lw/io/serializer/formatter.h"
#include "lw/io/serializer/parser.h"
#include "lw/mime/mime.h"

namespace lw::mime {

class JSONSerializationFormatter: public io::SerializationFormatter {
public:
  JSONSerializationFormatter(std::ostream& output);

  void put_null() override;
  void put_boolean(bool boolean) override;
  void put_char(char c) override;
  void put_signed_integer(std::int64_t number) override;
  void put_unsigned_integer(std::uint64_t number) override;
  void put_floating_point(double number) override;
  void put_string(std::string_view str) override;

  void start_list() override;
  void end_list() override;
  void start_object() override;
  void end_object() override;
  void start_pair_key() override;
  void end_pair_key() override;
  void end_pair() override;

private:
  enum class State {
    VALUE,
    LIST,
    LIST_VALUE,
    OBJECT,
    OBJECT_VALUE,
    KEY_STARTED,
    KEY_ADDED,
    KEY_ENDED
  };

  void _maybe_comma();
  void _check_can_add_value(
    const std::experimental::source_location& loc =
      std::experimental::source_location::current()
  );
  void _check_can_add_string(
    const std::experimental::source_location& loc =
      std::experimental::source_location::current()
  );
  void _value_added();
  void _string_added();
  void _add_literal(std::string_view value);
  void _put_char(char c);

  std::stack<State> _state;
  std::ostream& _output;
};

// -------------------------------------------------------------------------- //

class JSONDeserializationParser: public io::DeserializationParser {
public:
  std::unique_ptr<io::DeserializationToken> parse(
    std::string_view str
  ) const override;
};

// -------------------------------------------------------------------------- //

class JSONSerializer: public MimeSerializer {
public:
  std::unique_ptr<io::SerializationFormatter> make_formatter(
    std::ostream& output
  ) override {
    return std::make_unique<JSONSerializationFormatter>(output);
  }

  std::unique_ptr<io::DeserializationParser> make_parser() override {
    return std::make_unique<JSONDeserializationParser>();
  }
};

}
