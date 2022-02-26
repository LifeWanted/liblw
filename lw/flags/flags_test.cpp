#include "lw/flags/flags.h"

#include <sstream>
#include <string>
#include <string_view>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "lw/err/canonical.h"
#include "lw/flags/format.h"

LW_FLAG(bool, bool_true, true, "A true bool.");
LW_FLAG(bool, bool_false, false, "A false bool.");
LW_FLAG(
  int, answer_to_life, 42, "The answer to life, the universe, and everything."
);
LW_FLAG(float, pi, 3.14f, "Close enough...");
LW_FLAG(double, moar_pi, 3.1416, "Closer enough...");
LW_FLAG(std::string, str, "Lorem ipsum", "Completely original value.");

LW_FLAG(int, change_me, 123, "This value will be changed by tests.");
LW_FLAG(bool, change_me_bool, false, "This value will be changed by tests.");
LW_FLAG(bool, enable_me, false, "This value will be changed by tests.");
LW_FLAG(std::string, change_me_str, "", "This value will be changed by tests.");

// These flags should not compile.
// LW_FLAG(const int, const_not_allowed, 42, "Flag types must not be const.");
// LW_FLAG(volatile int, vol_not_allowed, 42, "Flags must not be volatile.");
// LW_FLAG(int&, ref_not_allowed, 42, "Flag types must not be reference.");
// LW_FLAG(int*, ptr_not_allowed, nullptr, "Flag types must not be pointers.");
// LW_FLAG(std::string_view, no_str_view, "", "Strings must use std::string.");

namespace other {

struct CustomType {
  int x = 0;
  int y = 0;
};

}

template <>
struct lw::cli::FlagTraits<other::CustomType> {
  std::string format_method(const other::CustomType& value) const {
    return std::to_string(value.x) + "," + std::to_string(value.y);
  }
  other::CustomType parse_method(std::string_view value) const {
    std::size_t comma_pos = value.find(',');
    return {
      .x = lw::cli::parse<int>({value.data(), comma_pos}),
      .y = lw::cli::parse<int>({
        value.data() + comma_pos + 1,
        value.size() - comma_pos - 1
      })
    };
  }
};

LW_FLAG(
  other::CustomType, custom_flag, (other::CustomType{.x = 0, .y = 1}),
  "This is a custom flag type."
);

namespace lw::cli {
namespace {

using ::testing::MatchesRegex;

TEST(FlagsExists, FalseForFlagsThatAreNotRegistered) {
  EXPECT_FALSE(flags_exists("this flag does not exist"));
}

TEST(FlagsExists, TrueForFlagsThatAreRegistered) {
  EXPECT_TRUE(flags_exists("bool_true"));
}

TEST(FlagsExists, ConvertsUnderscoresToDashes) {
  // Flag was registered with _, but we're testing with a -.
  EXPECT_TRUE(flags_exists("bool-true"));
}

// -------------------------------------------------------------------------- //

TEST(FlagsCliSet, ChangesFlagsByName) {
  flags::change_me = 1234;
  flags_cli_set("change_me", "4321", nullptr, 0);
  EXPECT_EQ(flags::change_me, 4321);
}

TEST(FlagsCliSet, ConvertsUnderscoresToDashes) {
  flags::change_me = 1234;
  flags_cli_set("change-me", "4321", nullptr, 0);
  EXPECT_EQ(flags::change_me, 4321);
}

TEST(FlagsCliSet, ChecksFlagsExist) {
  EXPECT_THROW(
    flags_cli_set("this flag does not exist", "foobar", nullptr, 0),
    InvalidArgument
  );
}

TEST(FlagsCliSet, DefaultsBooleansToYesValue) {
  flags::change_me_bool = false;
  EXPECT_FALSE(flags_cli_set("change-me-bool", std::nullopt, nullptr, 0));
  EXPECT_EQ(flags::change_me_bool, true);
}

TEST(FlagsCliSet, DoesNotDefaultOthersToYesValue) {
  flags::change_me_str = "";
  int argc = 1;
  const char* argv[] = {"--some-other-arg"};
  EXPECT_THROW(
    flags_cli_set("change-me-str", std::nullopt, argv, argc),
    InvalidArgument
  );
}

TEST(FlagsCliSet, NonBooleanTakesNextArgumentAsValue) {
  flags::change_me = 1234;
  int argc = 1;
  const char* argv[] = {"42"};
  EXPECT_TRUE(flags_cli_set("change-me", std::nullopt, argv, argc));
  EXPECT_EQ(flags::change_me, 42);
}

TEST(FlagsCliSet, BooleanDoesNotTakeNextArgumentAsValue) {
  flags::change_me_bool = true;
  int argc = 1;
  const char* argv[] = {"no"};
  EXPECT_FALSE(flags_cli_set("change-me-bool", std::nullopt, argv, argc));
  EXPECT_TRUE(flags::change_me_bool);

  flags::change_me_bool = false;
  const char* argv2[] = {"yes"};
  EXPECT_FALSE(flags_cli_set("change-me-bool", std::nullopt, argv2, argc));
  EXPECT_TRUE(flags::change_me_bool); // Booleans default to true.

  flags::change_me_bool = false;
  const char* argv3[] = {"gibberish"};
  EXPECT_FALSE(flags_cli_set("change-me-bool", std::nullopt, argv3, argc));
  EXPECT_TRUE(flags::change_me_bool); // Booleans default to true.
}

TEST(FlagsCliSet, DisablesWithNo) {
  flags::change_me_bool = true;
  int argc = 0;
  const char* argv[] = {};
  EXPECT_FALSE(flags_cli_set("nochange_me_bool", std::nullopt, argv, argc));
  EXPECT_FALSE(flags::change_me_bool);
}

TEST(FlagsCliSet, DisablesWithNoAndUnderscore) {
  flags::change_me_bool = true;
  int argc = 0;
  const char* argv[] = {};
  EXPECT_FALSE(flags_cli_set("no_change_me_bool", std::nullopt, argv, argc));
  EXPECT_FALSE(flags::change_me_bool);
}

TEST(FlagsCliSet, DisablesWithDisable) {
  flags::enable_me = true;
  int argc = 0;
  const char* argv[] = {};
  EXPECT_FALSE(flags_cli_set("disable_me", std::nullopt, argv, argc));
  EXPECT_FALSE(flags::enable_me);
}

TEST(FlagsCliSet, ParsesCustomTypes) {
  flags::custom_flag = other::CustomType{.x = 0, .y = 1};
  int argc = 0;
  const char* argv[] = {};
  EXPECT_FALSE(flags_cli_set("custom_flag", "2,3", argv, argc));
  EXPECT_EQ(flags::custom_flag.value().x, 2);
  EXPECT_EQ(flags::custom_flag.value().y, 3);

  EXPECT_EQ(flags::custom_flag.default_value_string(), "0,1");
}

// -------------------------------------------------------------------------- //

TEST(PrintFlags, PrintsFlagDescriptions) {
  std::stringstream out;
  cli::print_flags(out);

  EXPECT_THAT(
    out.str(),
    MatchesRegex(".*The answer to life, the universe, and everything.*")
  );
}

TEST(PrintFlags, PrintsInAlphabeticalOrder) {
  std::stringstream out;
  cli::print_flags(out);

  EXPECT_THAT(
    out.str(),
    MatchesRegex(
      ".*answer_to_life"
      ".*bool_false"
      ".*bool_true"
      ".*change_me"
      ".*change_me_bool"
      ".*change_me_str"
      ".*moar_pi"
      ".*pi.*"
    )
  );
}

// -------------------------------------------------------------------------- //

TEST(FlagsInstances, ConstructorMakesFlagsExist) {
  EXPECT_FALSE(flags_exists("this flag is created in this test"));
  Flag<int> flag{"", "this flag is created in this test", 1, ""};
  EXPECT_TRUE(flags_exists("this flag is created in this test"));
}

TEST(FlagsInstances, ConstructorConvertsUnderscoresToDashes) {
  EXPECT_FALSE(flags_exists("this-flag-is-created-in-this-test"));
  Flag<int> flag{"", "this-flag-is-created-in-this-test", 1, ""};
  EXPECT_TRUE(flags_exists("this-flag-is-created-in-this-test"));
  EXPECT_TRUE(flags_exists("this_flag_is_created_in_this_test"));
}

TEST(FlagsInstances, ImplicitlyConvertibleToValue) {
  EXPECT_TRUE(flags::bool_true);
  EXPECT_FALSE(flags::bool_false);
  EXPECT_EQ(flags::answer_to_life, 42);
  EXPECT_FLOAT_EQ(flags::pi, 3.14);
  EXPECT_DOUBLE_EQ(flags::moar_pi, 3.1416);
  EXPECT_EQ(flags::str, "Lorem ipsum");
}

TEST(FlagsInstances, CanChangeValue) {
  flags::change_me = 1234;
  EXPECT_EQ(flags::change_me, 1234);
  flags::change_me = 4321;
  EXPECT_EQ(flags::change_me, 4321);
}

}
}
