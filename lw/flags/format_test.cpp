#include "lw/flags/format.h"

#include "gtest/gtest.h"
#include "lw/err/canonical.h"

namespace lw::cli {
namespace {

TEST(FlagCli, FormatBool) {
  EXPECT_EQ(format(true), "true");
  EXPECT_EQ(format(false), "false");
}

TEST(FlagCli, ParseBoolTrue) {
  EXPECT_TRUE(parse<bool>("true"));
  EXPECT_TRUE(parse<bool>("yes"));
  EXPECT_TRUE(parse<bool>("1"));
}

TEST(FlagCli, ParseBoolFalse) {
  EXPECT_FALSE(parse<bool>("false"));
  EXPECT_FALSE(parse<bool>("no"));
  EXPECT_FALSE(parse<bool>("0"));
}

TEST(FlagCli, ParseBoolInvalid) {
  EXPECT_THROW(parse<bool>("gibberish"), InvalidArgument);
}

// -------------------------------------------------------------------------- //

TEST(FlagCli, FormatInt) {
  EXPECT_EQ(format((unsigned short)8080), "8080");
  EXPECT_EQ(format(-321), "-321");
}

TEST(FlagCli, ParseInt) {
  EXPECT_EQ(parse<unsigned short>("8080"), 8080);
  EXPECT_EQ(parse<int>("-321"), -321);
}

TEST(FlagCli, ParseIntInvalid) {
  EXPECT_THROW(parse<unsigned short>("1000000"), InvalidArgument);
  EXPECT_THROW(parse<unsigned int>("-321"), InvalidArgument);
  EXPECT_THROW(parse<int>("123 gibberish"), InvalidArgument);
  EXPECT_THROW(parse<long long int>("gibberish"), InvalidArgument);
}

// -------------------------------------------------------------------------- //

TEST(FlagCli, FormatFloat) {
  // TODO: Find a better way to encode floats to remove this grossness.
  EXPECT_EQ(format(123.456f), "123.456001"); // Floats are bad, m'kay.
  EXPECT_EQ(format(-654.321), "-654.321000");
}

TEST(FlagCli, ParseFloat) {
  EXPECT_EQ(parse<float>("123.456"), 123.456f);
  EXPECT_EQ(parse<double>("-654.321"), -654.321);
  EXPECT_EQ(parse<long double>("4.321E4"), 43210);
}

TEST(FlagCli, ParseFloatInvalid) {
  EXPECT_THROW(parse<float>("4E+38"), InvalidArgument);
  EXPECT_THROW(parse<double>("123 gibberish"), InvalidArgument);
  EXPECT_THROW(parse<long double>("gibberish"), InvalidArgument);
}

// -------------------------------------------------------------------------- //

TEST(FlagCli, FormatString) {
  EXPECT_EQ(format("foobar"), "foobar");
  EXPECT_EQ(format("fizz\\bang"), "fizz\\\\bang");
  EXPECT_EQ(format("gleep\"glorp"), "gleep\\\"glorp");
}

TEST(FlagCli, ParseString) {
  EXPECT_EQ(parse<std::string>("foobar"), "foobar");
  EXPECT_EQ(parse<std::string>("fizz\\\\bang"), "fizz\\bang");
  EXPECT_EQ(parse<std::string>("gleep\\\"glorp"), "gleep\"glorp");
}

}
}
