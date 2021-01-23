#pragma once

#include "gmock/gmock.h"
#include "lw/io/serializer/formatter.h"

namespace lw::io::testing {

class MockFormatter: public SerializationFormatter {
public:

  MOCK_METHOD(void, put_null, (), (override));
  MOCK_METHOD(void, put_boolean, (bool boolean), (override));
  MOCK_METHOD(void, put_char, (char c), (override));
  MOCK_METHOD(void, put_signed_integer, (std::int64_t number), (override));
  MOCK_METHOD(void, put_unsigned_integer, (std::uint64_t number), (override));
  MOCK_METHOD(void, put_floating_point, (double number), (override));
  MOCK_METHOD(void, put_string, (std::string_view str), (override));
  MOCK_METHOD(void, start_list, (), (override));
  MOCK_METHOD(void, end_list, (), (override));
  MOCK_METHOD(void, start_object, (), (override));
  MOCK_METHOD(void, end_object, (), (override));
  MOCK_METHOD(void, start_pair_key, (), (override));
  MOCK_METHOD(void, end_pair_key, (), (override));
  MOCK_METHOD(void, end_pair, (), (override));
};

}
