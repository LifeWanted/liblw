#include "lw/base/init.h"

#include "gtest/gtest.h"
#include "lw/err/canonical.h"
#include "lw/flags/flags.h"

LW_FLAG(int, flag, 42, "A flag to test with.");
LW_FLAG(int, other, 1234, "Another flag to test with.");
LW_FLAG(bool, boolean, true, "A flag defaulting to true.");

namespace lw {
namespace {

TEST(InitTest, ChecksArgsForNull) {
  int a;
  const char* b = "";
  EXPECT_THROW(init(nullptr, &b), InvalidArgument);
  EXPECT_THROW(init(&a, nullptr), InvalidArgument);
}

TEST(InitTest, SetsKnownFlagArguments) {
  int argc = 2;
  const char* argv[] = {"./binary/name", "--flag=69"};
  flags::flag = 1;
  EXPECT_TRUE(init(&argc, argv));
  EXPECT_EQ(flags::flag, 69);
}

TEST(InitTest, AllowsSeparatedFlagAndValue) {
  int argc = 3;
  const char* argv[] = {"./binary/name", "--flag", "69"};
  flags::flag = 1;
  EXPECT_TRUE(init(&argc, argv));
  EXPECT_EQ(flags::flag, 69);
}

TEST(InitTest, RemovesRecognizedFlags) {
  flags::flag = 1;
  flags::other = 2;
  int argc = 5;
  const char* argv[] = {
    "./binary/name", "--flag=69", "--other", "4321", "--this-is-not-used"
  };
  EXPECT_TRUE(init(&argc, argv));
  EXPECT_EQ(argc, 2);

  EXPECT_EQ(flags::flag, 69);
  EXPECT_EQ(flags::other, 4321);

  // Not using testing::ElementsAre becaue `argv` is a 5-element array and we
  // only care about the first values.
  EXPECT_EQ(std::string{argv[0]}, "./binary/name");
  EXPECT_EQ(std::string{argv[1]}, "--this-is-not-used");
}

TEST(InitTest, ParsesNoPrefixOnArgumentNames) {
  int argc = 2;
  const char* argv[] = {"./binary/name", "--no-boolean"};
  flags::boolean = true;

  EXPECT_TRUE(init(&argc, argv));
  EXPECT_EQ(argc, 1);
  EXPECT_FALSE(flags::boolean);
}

}
}
