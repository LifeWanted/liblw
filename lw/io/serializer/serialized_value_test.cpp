#include "lw/io/serializer/serialized_value.h"

#include <functional>
#include <string_view>

#include "gtest/gtest.h"
#include "lw/io/serializer/testing/mock_parser.h"
#include "lw/io/serializer/testing/tagged_types.h"

namespace lw::io {
namespace {

using ::testing::Return;
using ::testing::ReturnRef;

using Token = ::testing::StrictMock<::lw::io::testing::MockToken>;

TEST(SerializedValue, Null) {
  Token token;
  EXPECT_CALL(token, is_null()).WillOnce(Return(true));

  SerializedValue value{token};
  EXPECT_TRUE(value.is_null());
  EXPECT_EQ(value.as<std::nullptr_t>(), nullptr);
}

TEST(SerializedValue, Boolean) {
  Token token;
  EXPECT_CALL(token, is_boolean()).WillOnce(Return(true));
  EXPECT_CALL(token, get_boolean()).WillOnce(Return(false));

  SerializedValue value{token};
  EXPECT_TRUE(value.is_boolean());
  EXPECT_FALSE(value.as<bool>());
}

TEST(SerializedValue, Char) {
  Token token;
  EXPECT_CALL(token, is_char()).WillOnce(Return(true));
  EXPECT_CALL(token, get_char()).WillOnce(Return('f'));

  SerializedValue value{token};
  EXPECT_TRUE(value.is_char());
  EXPECT_EQ(value.as<char>(), 'f');
}

TEST(SerializedValue, SignedInteger) {
  Token token;
  EXPECT_CALL(token, is_signed_integer()).WillOnce(Return(true));
  EXPECT_CALL(token, get_signed_integer()).WillOnce(Return(-42));

  SerializedValue value{token};
  EXPECT_TRUE(value.is_signed_integer());
  EXPECT_EQ(value.as<int>(), -42);
}

TEST(SerializedValue, UnsignedInteger) {
  Token token;
  EXPECT_CALL(token, is_unsigned_integer()).WillOnce(Return(true));
  EXPECT_CALL(token, get_unsigned_integer()).WillOnce(Return(42));

  SerializedValue value{token};
  EXPECT_TRUE(value.is_unsigned_integer());
  EXPECT_EQ(value.as<std::size_t>(), 42);
}

TEST(SerializedValue, FloatingPoint) {
  Token token;
  EXPECT_CALL(token, is_floating_point()).WillOnce(Return(true));
  EXPECT_CALL(token, get_floating_point()).WillOnce(Return(3.14f));

  SerializedValue value{token};
  EXPECT_TRUE(value.is_floating_point());
  EXPECT_EQ(value.as<float>(), 3.14f);
}

TEST(SerializedValue, String) {
  Token token;
  EXPECT_CALL(token, is_string()).WillOnce(Return(true));
  EXPECT_CALL(token, get_string()).WillOnce(Return("foobar"));

  SerializedValue value{token};
  EXPECT_TRUE(value.is_string());
  EXPECT_EQ(value.as<std::string_view>(), "foobar");
}

TEST(SerializedValue, Object) {
  Token keyA;
  EXPECT_CALL(keyA, get_signed_integer).WillOnce(Return(10));
  Token keyB;
  EXPECT_CALL(keyB, get_signed_integer).WillOnce(Return(11));
  Token keyC;
  EXPECT_CALL(keyC, get_signed_integer).WillOnce(Return(12));

  Token token;
  EXPECT_CALL(token, is_object()).WillOnce(Return(true));
  EXPECT_CALL(token, get_key(std::string_view{"a"})).WillOnce(ReturnRef(keyA));
  EXPECT_CALL(token, get_key(std::string_view{"b"})).WillOnce(ReturnRef(keyB));
  EXPECT_CALL(token, get_key(std::string_view{"c"})).WillOnce(ReturnRef(keyC));

  SerializedValue value{token};
  EXPECT_TRUE(value.is_object());
  EXPECT_EQ(
    value.as<testing::ObjectTagged>(),
    (testing::ObjectTagged{.a = 10, .b = 11, .c = 12})
  );
}

TEST(SerializedValue, List) {
  Token index0;
  EXPECT_CALL(index0, get_signed_integer).WillOnce(Return(10));
  Token index1;
  EXPECT_CALL(index1, get_signed_integer).WillOnce(Return(11));
  Token index2;
  EXPECT_CALL(index2, get_signed_integer).WillOnce(Return(12));

  Token token;
  EXPECT_CALL(token, is_list()).WillOnce(Return(true));
  EXPECT_CALL(token, get_index(0)).WillOnce(ReturnRef(index0));
  EXPECT_CALL(token, get_index(1)).WillOnce(ReturnRef(index1));
  EXPECT_CALL(token, get_index(2)).WillOnce(ReturnRef(index2));

  SerializedValue value{token};
  EXPECT_TRUE(value.is_list());
  EXPECT_EQ(
    value.as<testing::ListTagged>(),
    (testing::ListTagged{.a = 10, .b = 11, .c = 12})
  );
}

}
}
