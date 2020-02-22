#include "lw/base/strings.h"

#include <string>
#include <string_view>

#include "gtest/gtest.h"

namespace lw {
namespace {

TEST(CaseInsensitiveHashTest, EmptyStringsHashTheSame) {
  const CaseInsensitiveHash<const char*> hasher;
  EXPECT_EQ(hasher(""), hasher(""));
}

TEST(CaseInsensitiveHashTest, SameStringsHashTheSame) {
  const CaseInsensitiveHash<const char*> hasher;
  EXPECT_EQ(hasher("foobar"), hasher("foobar"));
}

TEST(CaseInsensitiveHashTest, DifferentCasesHashTheSame) {
  const CaseInsensitiveHash<const char*> hasher;
  EXPECT_EQ(hasher("foobar"), hasher("Foobar"));
  EXPECT_EQ(hasher("foobar"), hasher("FooBar"));
  EXPECT_EQ(hasher("foobar"), hasher("FOObar"));
  EXPECT_EQ(hasher("foobar"), hasher("fooBAR"));
  EXPECT_EQ(hasher("foobar"), hasher("FOOBAR"));
}

TEST(CaseInsensitiveHashTest, DifferentStringsHashDifferent) {
  const CaseInsensitiveHash<const char*> hasher;
  EXPECT_NE(hasher("foobar"), hasher("fizzbang"));
}

TEST(CaseInsensitiveHashTest, HashIsNotIncremental) {
  const CaseInsensitiveHash<const char*> hasher;
  EXPECT_NE(hasher("aa") + 1, hasher("ab"));
}

TEST(CaseInsensitiveHashTest, DifferentClassesHashTheSame) {
  const CaseInsensitiveHash<const char*> char_hasher;
  const CaseInsensitiveHash<std::string> str_hasher;
  const CaseInsensitiveHash<std::string_view> view_hasher;

  const char* char_foobar = "foobar";
  const char* char_fizzbang = "fizzbang";
  const std::string str_foobar = "FOObar";
  const std::string str_fizzbang = "FIZZbang";
  const std::string_view view_foobar = "fooBAR";
  const std::string_view view_fizzbang = "fizzBANG";

  EXPECT_EQ(char_hasher(char_foobar), str_hasher(str_foobar));
  EXPECT_EQ(char_hasher(char_foobar), view_hasher(view_foobar));
  EXPECT_EQ(str_hasher(str_foobar), view_hasher(view_foobar));

  EXPECT_EQ(char_hasher(char_fizzbang), str_hasher(str_fizzbang));
  EXPECT_EQ(char_hasher(char_fizzbang), view_hasher(view_fizzbang));
  EXPECT_EQ(str_hasher(str_fizzbang), view_hasher(view_fizzbang));

  EXPECT_NE(char_hasher(char_fizzbang), str_hasher(str_foobar));
  EXPECT_NE(char_hasher(char_fizzbang), view_hasher(view_foobar));
  EXPECT_NE(str_hasher(str_fizzbang), view_hasher(view_foobar));

  EXPECT_NE(char_hasher(char_foobar), str_hasher(str_fizzbang));
  EXPECT_NE(char_hasher(char_foobar), view_hasher(view_fizzbang));
  EXPECT_NE(str_hasher(str_foobar), view_hasher(view_fizzbang));
}

// -------------------------------------------------------------------------- //

TEST(CaseInsensitiveEqualCString, EmptyStringsAreEqual) {
  const CaseInsensitiveEqual<const char*> eql;
  EXPECT_TRUE(eql("", ""));
}

TEST(CaseInsensitiveEqualCString, SameStringsAreEqual) {
  const CaseInsensitiveEqual<const char*> eql;
  EXPECT_TRUE(eql("foobar", "foobar"));
}

TEST(CaseInsensitiveEqualCString, DifferentCasesAreEqual) {
  const CaseInsensitiveEqual<const char*> eql;
  EXPECT_TRUE(eql("foobar", "Foobar"));
  EXPECT_TRUE(eql("Foobar", "foobar"));
  EXPECT_TRUE(eql("foobar", "FOObar"));
  EXPECT_TRUE(eql("FOObar", "foobar"));
  EXPECT_TRUE(eql("foobar", "fooBAR"));
  EXPECT_TRUE(eql("fooBAR", "foobar"));
  EXPECT_TRUE(eql("foobar", "FOOBAR"));
  EXPECT_TRUE(eql("FOOBAR", "foobar"));
}

TEST(CaseInsensitiveEqualCString, DifferentLengthsAreNotEqual) {
  const CaseInsensitiveEqual<const char*> eql;
  EXPECT_FALSE(eql("foobar", "foobar2"));
  EXPECT_FALSE(eql("foobar2", "foobar"));
}

TEST(CaseInsensitiveEqualCString, DifferentStringsAreNotEqual) {
  const CaseInsensitiveEqual<const char*> eql;
  EXPECT_FALSE(eql("foobar", "foocar"));
  EXPECT_FALSE(eql("foocar", "foobar"));
}

TEST(CaseInsensitiveEqualStringView, EmptyStringsAreEqual) {
  const CaseInsensitiveEqual<std::string_view> eql;
  EXPECT_TRUE(eql("", ""));
}

TEST(CaseInsensitiveEqualStringView, SameStringsAreEqual) {
  const CaseInsensitiveEqual<std::string_view> eql;
  EXPECT_TRUE(eql("foobar", "foobar"));
}

TEST(CaseInsensitiveEqualStringView, DifferentCasesAreEqual) {
  const CaseInsensitiveEqual<std::string_view> eql;
  EXPECT_TRUE(eql("foobar", "Foobar"));
  EXPECT_TRUE(eql("Foobar", "foobar"));
  EXPECT_TRUE(eql("foobar", "FOObar"));
  EXPECT_TRUE(eql("FOObar", "foobar"));
  EXPECT_TRUE(eql("foobar", "fooBAR"));
  EXPECT_TRUE(eql("fooBAR", "foobar"));
  EXPECT_TRUE(eql("foobar", "FOOBAR"));
  EXPECT_TRUE(eql("FOOBAR", "foobar"));
}

TEST(CaseInsensitiveEqualStringView, DifferentLengthsAreNotEqual) {
  const CaseInsensitiveEqual<std::string_view> eql;
  EXPECT_FALSE(eql("foobar", "foobar2"));
  EXPECT_FALSE(eql("foobar2", "foobar"));
}

TEST(CaseInsensitiveEqualStringView, DifferentStringsAreNotEqual) {
  const CaseInsensitiveEqual<std::string_view> eql;
  EXPECT_FALSE(eql("foobar", "foocar"));
  EXPECT_FALSE(eql("foocar", "foobar"));
}

}
}
