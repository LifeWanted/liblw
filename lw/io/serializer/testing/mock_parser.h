#pragma once

#include "gmock/gmock.h"
#include "lw/io/serializer/parser.h"

namespace lw::io::testing {

class MockToken: public DeserializationToken {
public:
  MOCK_METHOD(std::size_t, size, (), (const override));
  MOCK_METHOD(bool, is_null, (), (const override));
  MOCK_METHOD(bool, is_boolean, (), (const override));
  MOCK_METHOD(bool, is_char, (), (const override));
  MOCK_METHOD(bool, is_signed_integer, (), (const override));
  MOCK_METHOD(bool, is_unsigned_integer, (), (const override));
  MOCK_METHOD(bool, is_floating_point, (), (const override));
  MOCK_METHOD(bool, is_string, (), (const override));
  MOCK_METHOD(bool, is_list, (), (const override));
  MOCK_METHOD(bool, is_object, (), (const override));
  MOCK_METHOD(bool, get_boolean, (), (const override));
  MOCK_METHOD(char, get_char, (), (const override));
  MOCK_METHOD(std::int64_t, get_signed_integer, (), (const override));
  MOCK_METHOD(std::uint64_t, get_unsigned_integer, (), (const override));
  MOCK_METHOD(double, get_floating_point, (), (const override));
  MOCK_METHOD(std::string_view, get_string, (), (const override));
  MOCK_METHOD(bool, has_index, (std::size_t idx), (const override));
  MOCK_METHOD(
    const DeserializationToken&,
    get_index,
    (std::size_t idx),
    (const override)
  );
  MOCK_METHOD(bool, has_key, (std::string_view key), (const override));
  MOCK_METHOD(
    const DeserializationToken&,
    get_key,
    (std::string_view key),
    (const override)
  );
};

}
