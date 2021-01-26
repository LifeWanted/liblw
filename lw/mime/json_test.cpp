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

}
}
