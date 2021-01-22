#include "lw/io/serializer/serializer.h"

#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <map>

#include "gtest/gtest.h"
#include "lw/io/serializer/testing/mock_formatter.h"

namespace lw::io {
namespace {

using ::lw::io::testing::MockFormatter;
using ::testing::StrictMock;

TEST(Serializer, WriteNull) {
  auto formatter = std::make_unique<StrictMock<MockFormatter>>();
  EXPECT_CALL(*formatter, put_null()).Times(1);

  Serializer s{std::move(formatter)};
  s.write(nullptr);
}

TEST(Serializer, WriteBool) {
  auto formatter = std::make_unique<StrictMock<MockFormatter>>();
  EXPECT_CALL(*formatter, put_boolean(true)).Times(1);
  EXPECT_CALL(*formatter, put_boolean(false)).Times(1);

  Serializer s{std::move(formatter)};
  s.write(true);
  s.write(false);
}

TEST(Serializer, WriteChar) {
  auto formatter = std::make_unique<StrictMock<MockFormatter>>();
  EXPECT_CALL(*formatter, put_char('c')).Times(1);

  Serializer s{std::move(formatter)};
  s.write('c');
}

TEST(Serializer, WriteNumber) {
  auto formatter = std::make_unique<StrictMock<MockFormatter>>();
  EXPECT_CALL(*formatter, put_signed_integer(-1)).Times(1);
  EXPECT_CALL(*formatter, put_unsigned_integer(42u)).Times(1);
  EXPECT_CALL(*formatter, put_floating_point(3.14)).Times(1);
  EXPECT_CALL(*formatter, put_floating_point(1.23f)).Times(1);

  Serializer s{std::move(formatter)};
  s.write(-1);
  s.write(42u);
  s.write(3.14);
  s.write(1.23f);
}

TEST(Serializer, WriteString) {
  auto formatter = std::make_unique<StrictMock<MockFormatter>>();
  EXPECT_CALL(*formatter, put_string(std::string_view{"foo"})).Times(1);
  EXPECT_CALL(*formatter, put_string(std::string_view{"bar"})).Times(1);
  EXPECT_CALL(*formatter, put_string(std::string_view{"bang"})).Times(1);

  Serializer s{std::move(formatter)};
  s.write("foo");
  s.write(std::string_view{"bar"});
  s.write(std::string{"bang"});
}

TEST(Serializer, WriteList) {
  auto formatter = std::make_unique<StrictMock<MockFormatter>>();
  EXPECT_CALL(*formatter, put_string(std::string_view{"foo"})).Times(3);
  EXPECT_CALL(*formatter, start_list()).Times(1);
  EXPECT_CALL(*formatter, end_list()).Times(1);

  Serializer s{std::move(formatter)};
  s.write(std::vector<std::string_view>{{"foo", "foo", "foo"}});
}

TEST(Serializer, WriteListOfList) {
  auto formatter = std::make_unique<StrictMock<MockFormatter>>();
  EXPECT_CALL(*formatter, put_string(std::string_view{"foo"})).Times(3);
  EXPECT_CALL(*formatter, put_string(std::string_view{"bar"})).Times(2);
  EXPECT_CALL(*formatter, start_list()).Times(3);
  EXPECT_CALL(*formatter, end_list()).Times(3);

  Serializer s{std::move(formatter)};
  s.write(
    std::vector<std::vector<std::string_view>>{
      {"foo", "foo", "foo"},
      {"bar", "bar"}
    }
  );
}

TEST(Serializer, WriteObject) {
  auto formatter = std::make_unique<StrictMock<MockFormatter>>();
  EXPECT_CALL(*formatter, put_string(std::string_view{"foo"})).Times(1);
  EXPECT_CALL(*formatter, put_string(std::string_view{"bar"})).Times(1);
  EXPECT_CALL(*formatter, put_string(std::string_view{"fizz"})).Times(1);
  EXPECT_CALL(*formatter, put_string(std::string_view{"bang"})).Times(1);
  EXPECT_CALL(*formatter, start_object()).Times(1);
  EXPECT_CALL(*formatter, start_pair_key()).Times(2);
  EXPECT_CALL(*formatter, end_pair_key()).Times(2);
  EXPECT_CALL(*formatter, end_pair()).Times(2);
  EXPECT_CALL(*formatter, end_object()).Times(1);

  Serializer s{std::move(formatter)};
  s.write(
    std::map<std::string_view, std::string_view>{
      {"foo", "bar"},
      {"fizz", "bang"}
    }
  );
}

}
}
