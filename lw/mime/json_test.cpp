#include "lw/mime/json.h"

#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lw/io/serializer/serializer.h"
#include "lw/io/serializer/testing/tagged_types.h"

namespace lw::mime {
namespace {

using ::lw::io::testing::ListTagged;
using ::lw::io::testing::ObjectTagged;
using ::testing::MatchesRegex;

template <typename T>
std::string serialize(const T& value) {
  std::stringstream stream;
  JSONSerializer json;
  io::Serializer serializer{json.make_formatter(stream)};
  serializer.write(value);
  return stream.str();
}

std::unique_ptr<io::DeserializationToken> parse(std::string_view str) {
  JSONDeserializationParser parser;
  return parser.parse(str);
}

TEST(JSONSerializer, Null) {
  EXPECT_EQ(serialize(nullptr), "null");
}

TEST(JSONSerializer, Integer) {
  EXPECT_EQ(serialize(1), "1");
  EXPECT_EQ(serialize(-42), "-42");
  EXPECT_EQ(serialize(69u), "69");
}

TEST(JSONSerializer, Float) {
  EXPECT_EQ(serialize(3.14), "3.14");
  EXPECT_EQ(serialize(1.234f), "1.234");
}

TEST(JSONSerializer, String) {
  EXPECT_EQ(serialize('a'), R"("a")");
  EXPECT_EQ(serialize("foobar"), R"("foobar")");
  EXPECT_EQ(serialize("foo\\bar"), R"("foo\\bar")");
  EXPECT_EQ(serialize("foo\bar"), R"("foo\bar")");
  EXPECT_EQ(serialize("foo\"bar"), R"("foo\"bar")");
}

TEST(JSONSerializer, Boolean) {
  EXPECT_EQ(serialize(true), "true");
  EXPECT_EQ(serialize(false), "false");
}

TEST(JSONSerializer, List) {
  EXPECT_EQ(serialize(std::vector<int>{{1, 2, 3}}), "[1,2,3]");
}

TEST(JSONSerializer, ListTagged) {
  EXPECT_EQ(serialize(ListTagged{.a = 1, .b = 2, .c = 3}), "[1,2,3]");
}

TEST(JSONSerializer, Object) {
  EXPECT_THAT(
    serialize(std::map<std::string, int>{{"foo", 1}, {"bar", 2}}),
    MatchesRegex(R"(\{"foo":1,"bar":2\}|\{"bar":2,"foo":1\})")
  );
}

TEST(JSONSerializer, ObjectTagged) {
  EXPECT_EQ(
    serialize(ObjectTagged{.a = 1, .b = 2, .c = 3}),
    R"({"a":1,"b":2,"c":3})"
  );
}

// -------------------------------------------------------------------------- //

TEST(JSONParser, Null) {
  auto token = parse("null");
  ASSERT_NE(token, nullptr);

  EXPECT_TRUE(token->is_null());
  EXPECT_FALSE(token->is_boolean());
  EXPECT_FALSE(token->is_char());
  EXPECT_FALSE(token->is_signed_integer());
  EXPECT_FALSE(token->is_unsigned_integer());
  EXPECT_FALSE(token->is_floating_point());
  EXPECT_FALSE(token->is_string());
  EXPECT_FALSE(token->is_list());
  EXPECT_FALSE(token->is_object());

  EXPECT_FALSE(token->has_index(0));
  EXPECT_FALSE(token->has_key("null"));
}

TEST(JSONParser, Integer) {
  auto token = parse("42");
  ASSERT_NE(token, nullptr);

  EXPECT_FALSE(token->is_null());
  EXPECT_FALSE(token->is_boolean());
  EXPECT_FALSE(token->is_char());
  EXPECT_TRUE(token->is_signed_integer());
  EXPECT_TRUE(token->is_unsigned_integer());
  EXPECT_TRUE(token->is_floating_point());
  EXPECT_FALSE(token->is_string());
  EXPECT_FALSE(token->is_list());
  EXPECT_FALSE(token->is_object());

  EXPECT_FALSE(token->has_index(0));
  EXPECT_FALSE(token->has_key("42"));

  EXPECT_EQ(token->get_signed_integer(), 42);
  EXPECT_EQ(token->get_unsigned_integer(), 42u);
  EXPECT_EQ(token->get_floating_point(), 42.0);
}

TEST(JSONParser, NegativeSignedInteger) {
  auto token = parse("-42");
  ASSERT_NE(token, nullptr);

  EXPECT_FALSE(token->is_null());
  EXPECT_FALSE(token->is_boolean());
  EXPECT_FALSE(token->is_char());
  EXPECT_TRUE(token->is_signed_integer());
  EXPECT_FALSE(token->is_unsigned_integer());
  EXPECT_TRUE(token->is_floating_point());
  EXPECT_FALSE(token->is_string());
  EXPECT_FALSE(token->is_list());
  EXPECT_FALSE(token->is_object());

  EXPECT_FALSE(token->has_index(0));
  EXPECT_FALSE(token->has_key("-42"));

  EXPECT_EQ(token->get_signed_integer(), -42);
  EXPECT_EQ(token->get_floating_point(), -42.0);
}

TEST(JSONParser, PositiveSignedInteger) {
  auto token = parse("+42");
  ASSERT_NE(token, nullptr);

  EXPECT_FALSE(token->is_null());
  EXPECT_FALSE(token->is_boolean());
  EXPECT_FALSE(token->is_char());
  EXPECT_TRUE(token->is_signed_integer());
  EXPECT_TRUE(token->is_unsigned_integer());
  EXPECT_TRUE(token->is_floating_point());
  EXPECT_FALSE(token->is_string());
  EXPECT_FALSE(token->is_list());
  EXPECT_FALSE(token->is_object());

  EXPECT_FALSE(token->has_index(0));
  EXPECT_FALSE(token->has_key("+42"));

  EXPECT_EQ(token->get_signed_integer(), 42);
  EXPECT_EQ(token->get_unsigned_integer(), 42u);
  EXPECT_EQ(token->get_floating_point(), 42.0);
}

TEST(JSONParser, Float) {
  auto token = parse("3.14");
  ASSERT_NE(token, nullptr);

  EXPECT_FALSE(token->is_null());
  EXPECT_FALSE(token->is_boolean());
  EXPECT_FALSE(token->is_char());
  EXPECT_FALSE(token->is_signed_integer());
  EXPECT_FALSE(token->is_unsigned_integer());
  EXPECT_TRUE(token->is_floating_point());
  EXPECT_FALSE(token->is_string());
  EXPECT_FALSE(token->is_list());
  EXPECT_FALSE(token->is_object());

  EXPECT_FALSE(token->has_index(0));
  EXPECT_FALSE(token->has_key("3.14"));

  EXPECT_EQ(token->get_floating_point(), 3.14);
}

TEST(JSONParser, NegativeSignedFloat) {
  auto token = parse("-3.14");
  ASSERT_NE(token, nullptr);

  EXPECT_FALSE(token->is_null());
  EXPECT_FALSE(token->is_boolean());
  EXPECT_FALSE(token->is_char());
  EXPECT_FALSE(token->is_signed_integer());
  EXPECT_FALSE(token->is_unsigned_integer());
  EXPECT_TRUE(token->is_floating_point());
  EXPECT_FALSE(token->is_string());
  EXPECT_FALSE(token->is_list());
  EXPECT_FALSE(token->is_object());

  EXPECT_FALSE(token->has_index(0));
  EXPECT_FALSE(token->has_key("-3.14"));

  EXPECT_EQ(token->get_floating_point(), -3.14);
}

TEST(JSONParser, PositiveSignedFloat) {
  auto token = parse("+3.14");
  ASSERT_NE(token, nullptr);

  EXPECT_FALSE(token->is_null());
  EXPECT_FALSE(token->is_boolean());
  EXPECT_FALSE(token->is_char());
  EXPECT_FALSE(token->is_signed_integer());
  EXPECT_FALSE(token->is_unsigned_integer());
  EXPECT_TRUE(token->is_floating_point());
  EXPECT_FALSE(token->is_string());
  EXPECT_FALSE(token->is_list());
  EXPECT_FALSE(token->is_object());

  EXPECT_FALSE(token->has_index(0));
  EXPECT_FALSE(token->has_key("+3.14"));

  EXPECT_EQ(token->get_floating_point(), 3.14);
}

TEST(JSONParser, String) {
  auto token = parse(R"("foobar")");
  ASSERT_NE(token, nullptr);

  EXPECT_FALSE(token->is_null());
  EXPECT_FALSE(token->is_boolean());
  EXPECT_FALSE(token->is_char());
  EXPECT_FALSE(token->is_signed_integer());
  EXPECT_FALSE(token->is_unsigned_integer());
  EXPECT_FALSE(token->is_floating_point());
  EXPECT_TRUE(token->is_string());
  EXPECT_FALSE(token->is_list());
  EXPECT_FALSE(token->is_object());

  EXPECT_FALSE(token->has_index(0));
  EXPECT_FALSE(token->has_key("foobar"));
  EXPECT_FALSE(token->has_key(R"("foobar")"));

  EXPECT_EQ(token->size(), 6);
  EXPECT_EQ(token->get_string(), "foobar");
}

TEST(JSONParser, StringEscapes) {
  auto token = parse(R"("\\\\")");
  ASSERT_NE(token, nullptr);

  EXPECT_FALSE(token->is_null());
  EXPECT_FALSE(token->is_boolean());
  EXPECT_FALSE(token->is_char());
  EXPECT_FALSE(token->is_signed_integer());
  EXPECT_FALSE(token->is_unsigned_integer());
  EXPECT_FALSE(token->is_floating_point());
  EXPECT_TRUE(token->is_string());
  EXPECT_FALSE(token->is_list());
  EXPECT_FALSE(token->is_object());

  EXPECT_FALSE(token->has_index(0));
  EXPECT_FALSE(token->has_key("\\\\"));
  EXPECT_FALSE(token->has_key(R"("\\\\")"));

  EXPECT_EQ(token->size(), 2);
  EXPECT_EQ(token->get_string(), "\\\\");
}

TEST(JSONParser, DecodesString) {
  auto token = parse(R"("\fooba\r")");
  ASSERT_NE(token, nullptr);

  EXPECT_FALSE(token->is_null());
  EXPECT_FALSE(token->is_boolean());
  EXPECT_FALSE(token->is_char());
  EXPECT_FALSE(token->is_signed_integer());
  EXPECT_FALSE(token->is_unsigned_integer());
  EXPECT_FALSE(token->is_floating_point());
  EXPECT_TRUE(token->is_string());
  EXPECT_FALSE(token->is_list());
  EXPECT_FALSE(token->is_object());

  EXPECT_FALSE(token->has_index(0));
  EXPECT_FALSE(token->has_key("\fooba\r"));
  EXPECT_FALSE(token->has_key(R"("\fooba\r")"));

  EXPECT_EQ(token->size(), 6);
  EXPECT_EQ(token->get_string(), "\fooba\r");
}

TEST(JSONParser, Char) {
  auto token = parse(R"("f")");
  ASSERT_NE(token, nullptr);

  EXPECT_FALSE(token->is_null());
  EXPECT_FALSE(token->is_boolean());
  EXPECT_TRUE(token->is_char());
  EXPECT_FALSE(token->is_signed_integer());
  EXPECT_FALSE(token->is_unsigned_integer());
  EXPECT_FALSE(token->is_floating_point());
  EXPECT_TRUE(token->is_string());
  EXPECT_FALSE(token->is_list());
  EXPECT_FALSE(token->is_object());

  EXPECT_FALSE(token->has_index(0));
  EXPECT_FALSE(token->has_key("f"));
  EXPECT_FALSE(token->has_key(R"("f")"));

  EXPECT_EQ(token->size(), 1);
  EXPECT_EQ(token->get_char(), 'f');
  EXPECT_EQ(token->get_string(), "f");
}

TEST(JSONParser, DecodesChar) {
  auto token = parse(R"("\n")");
  ASSERT_NE(token, nullptr);

  EXPECT_FALSE(token->is_null());
  EXPECT_FALSE(token->is_boolean());
  EXPECT_TRUE(token->is_char());
  EXPECT_FALSE(token->is_signed_integer());
  EXPECT_FALSE(token->is_unsigned_integer());
  EXPECT_FALSE(token->is_floating_point());
  EXPECT_TRUE(token->is_string());
  EXPECT_FALSE(token->is_list());
  EXPECT_FALSE(token->is_object());

  EXPECT_FALSE(token->has_index(0));
  EXPECT_FALSE(token->has_key("\n"));
  EXPECT_FALSE(token->has_key(R"("\n")"));

  EXPECT_EQ(token->size(), 1);
  EXPECT_EQ(token->get_char(), '\n');
  EXPECT_EQ(token->get_string(), "\n");
}

TEST(JSONParser, True) {
  auto token = parse("true");
  ASSERT_NE(token, nullptr);

  EXPECT_FALSE(token->is_null());
  EXPECT_TRUE(token->is_boolean());
  EXPECT_FALSE(token->is_char());
  EXPECT_FALSE(token->is_signed_integer());
  EXPECT_FALSE(token->is_unsigned_integer());
  EXPECT_FALSE(token->is_floating_point());
  EXPECT_FALSE(token->is_string());
  EXPECT_FALSE(token->is_list());
  EXPECT_FALSE(token->is_object());

  EXPECT_FALSE(token->has_index(0));
  EXPECT_FALSE(token->has_key("true"));

  EXPECT_TRUE(token->get_boolean());
}

TEST(JSONParser, False) {
  auto token = parse("false");
  ASSERT_NE(token, nullptr);

  EXPECT_FALSE(token->is_null());
  EXPECT_TRUE(token->is_boolean());
  EXPECT_FALSE(token->is_char());
  EXPECT_FALSE(token->is_signed_integer());
  EXPECT_FALSE(token->is_unsigned_integer());
  EXPECT_FALSE(token->is_floating_point());
  EXPECT_FALSE(token->is_string());
  EXPECT_FALSE(token->is_list());
  EXPECT_FALSE(token->is_object());

  EXPECT_FALSE(token->has_index(0));
  EXPECT_FALSE(token->has_key("false"));

  EXPECT_FALSE(token->get_boolean());
}

TEST(JSONParser, List) {
  auto token = parse(R"(["foo", 2, null])");
  ASSERT_NE(token, nullptr);

  EXPECT_FALSE(token->is_null());
  EXPECT_FALSE(token->is_boolean());
  EXPECT_FALSE(token->is_char());
  EXPECT_FALSE(token->is_signed_integer());
  EXPECT_FALSE(token->is_unsigned_integer());
  EXPECT_FALSE(token->is_floating_point());
  EXPECT_FALSE(token->is_string());
  EXPECT_TRUE(token->is_list());
  EXPECT_FALSE(token->is_object());

  EXPECT_TRUE(token->has_index(0));
  EXPECT_TRUE(token->has_index(1));
  EXPECT_TRUE(token->has_index(2));
  EXPECT_FALSE(token->has_index(3));
  EXPECT_FALSE(token->has_key("foo"));
  EXPECT_FALSE(token->has_key(R"("foo")"));
  EXPECT_FALSE(token->has_key("2"));
  EXPECT_FALSE(token->has_key("null"));

  EXPECT_EQ(token->size(), 3);
  EXPECT_TRUE(token->get_index(0).is_string());
  EXPECT_TRUE(token->get_index(1).is_unsigned_integer());
  EXPECT_TRUE(token->get_index(2).is_null());
  EXPECT_EQ(token->get_index(0).get_string(), "foo");
  EXPECT_EQ(token->get_index(1).get_unsigned_integer(), 2);
}

TEST(JSONParser, EmptyList) {
  auto token = parse(R"([])");
  ASSERT_NE(token, nullptr);

  EXPECT_FALSE(token->is_null());
  EXPECT_FALSE(token->is_boolean());
  EXPECT_FALSE(token->is_char());
  EXPECT_FALSE(token->is_signed_integer());
  EXPECT_FALSE(token->is_unsigned_integer());
  EXPECT_FALSE(token->is_floating_point());
  EXPECT_FALSE(token->is_string());
  EXPECT_TRUE(token->is_list());
  EXPECT_FALSE(token->is_object());

  EXPECT_FALSE(token->has_index(0));
  EXPECT_FALSE(token->has_key("[]"));

  EXPECT_EQ(token->size(), 0);
}

TEST(JSONParser, NestedList) {
  auto token = parse(R"([[1,2]])");
  ASSERT_NE(token, nullptr);

  EXPECT_FALSE(token->is_null());
  EXPECT_FALSE(token->is_boolean());
  EXPECT_FALSE(token->is_char());
  EXPECT_FALSE(token->is_signed_integer());
  EXPECT_FALSE(token->is_unsigned_integer());
  EXPECT_FALSE(token->is_floating_point());
  EXPECT_FALSE(token->is_string());
  EXPECT_TRUE(token->is_list());
  EXPECT_FALSE(token->is_object());

  EXPECT_TRUE(token->has_index(0));
  EXPECT_FALSE(token->has_index(1));
  EXPECT_FALSE(token->has_key("[[1,2]]"));
  EXPECT_FALSE(token->has_key("[1,2]"));
  EXPECT_FALSE(token->has_key("1"));
  EXPECT_FALSE(token->has_key("2"));

  EXPECT_EQ(token->size(), 1);
  EXPECT_TRUE(token->get_index(0).is_list());
  EXPECT_EQ(token->get_index(0).size(), 2);
  EXPECT_EQ(token->get_index(0).get_index(0).get_unsigned_integer(), 1);
  EXPECT_EQ(token->get_index(0).get_index(1).get_unsigned_integer(), 2);
}

TEST(JSONParser, Object) {
  auto token = parse(R"({"foo": "bar", "fizz":2})");
  ASSERT_NE(token, nullptr);

  EXPECT_FALSE(token->is_null());
  EXPECT_FALSE(token->is_boolean());
  EXPECT_FALSE(token->is_char());
  EXPECT_FALSE(token->is_signed_integer());
  EXPECT_FALSE(token->is_unsigned_integer());
  EXPECT_FALSE(token->is_floating_point());
  EXPECT_FALSE(token->is_string());
  EXPECT_FALSE(token->is_list());
  EXPECT_TRUE(token->is_object());

  EXPECT_FALSE(token->has_index(0));
  EXPECT_FALSE(token->has_key(R"({"foo": "bar", "fizz":2})"));
  EXPECT_FALSE(token->has_key(R"("foo")"));
  EXPECT_FALSE(token->has_key(R"("bar")"));
  EXPECT_FALSE(token->has_key(R"("fizz")"));
  EXPECT_FALSE(token->has_key("bar"));
  EXPECT_FALSE(token->has_key("2"));
  EXPECT_TRUE(token->has_key("foo"));
  EXPECT_TRUE(token->has_key("fizz"));

  EXPECT_EQ(token->size(), 2);
  EXPECT_TRUE(token->get_key("foo").is_string());
  EXPECT_TRUE(token->get_key("fizz").is_unsigned_integer());
  EXPECT_EQ(token->get_key("foo").get_string(), "bar");
  EXPECT_EQ(token->get_key("fizz").get_unsigned_integer(), 2);
}

}
}
