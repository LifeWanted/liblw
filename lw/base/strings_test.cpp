#include "lw/base/strings.h"

#include <cctype>
#include <limits>
#include <string>
#include <string_view>
#include <unordered_map>

#include "gtest/gtest.h"

namespace lw {
namespace {

TEST(CaseInsensitiveHashTest, EmptyStringsHashTheSame) {
  const CaseInsensitiveHash hasher;
  EXPECT_EQ(hasher(""), hasher(""));
}

TEST(CaseInsensitiveHashTest, SameStringsHashTheSame) {
  const CaseInsensitiveHash hasher;
  EXPECT_EQ(hasher("foobar"), hasher("foobar"));
}

TEST(CaseInsensitiveHashTest, DifferentCasesHashTheSame) {
  const CaseInsensitiveHash hasher;
  EXPECT_EQ(hasher("foobar"), hasher("Foobar"));
  EXPECT_EQ(hasher("foobar"), hasher("FooBar"));
  EXPECT_EQ(hasher("foobar"), hasher("FOObar"));
  EXPECT_EQ(hasher("foobar"), hasher("fooBAR"));
  EXPECT_EQ(hasher("foobar"), hasher("FOOBAR"));
}

TEST(CaseInsensitiveHashTest, DifferentStringsHashDifferent) {
  const CaseInsensitiveHash hasher;
  EXPECT_NE(hasher("foobar"), hasher("fizzbang"));
}

TEST(CaseInsensitiveHashTest, HashIsNotIncremental) {
  const CaseInsensitiveHash hasher;
  EXPECT_NE(hasher("aa") + 1, hasher("ab"));
}

TEST(CaseInsensitiveHashTest, DifferentClassesHashTheSame) {
  const CaseInsensitiveHash hasher;

  const char* char_foobar = "foobar";
  const char* char_fizzbang = "fizzbang";
  const std::string str_foobar = "FOObar";
  const std::string str_fizzbang = "FIZZbang";
  const std::string_view view_foobar = "fooBAR";
  const std::string_view view_fizzbang = "fizzBANG";

  EXPECT_EQ(hasher(char_foobar), hasher(str_foobar));
  EXPECT_EQ(hasher(char_foobar), hasher(view_foobar));
  EXPECT_EQ(hasher(str_foobar), hasher(view_foobar));

  EXPECT_EQ(hasher(char_fizzbang), hasher(str_fizzbang));
  EXPECT_EQ(hasher(char_fizzbang), hasher(view_fizzbang));
  EXPECT_EQ(hasher(str_fizzbang), hasher(view_fizzbang));

  EXPECT_NE(hasher(char_fizzbang), hasher(str_foobar));
  EXPECT_NE(hasher(char_fizzbang), hasher(view_foobar));
  EXPECT_NE(hasher(str_fizzbang), hasher(view_foobar));

  EXPECT_NE(hasher(char_foobar), hasher(str_fizzbang));
  EXPECT_NE(hasher(char_foobar), hasher(view_fizzbang));
  EXPECT_NE(hasher(str_foobar), hasher(view_fizzbang));
}

// -------------------------------------------------------------------------- //

TEST(CaseInsensitiveEqualCString, EmptyStringsAreEqual) {
  const CaseInsensitiveEqual eql;
  EXPECT_TRUE(eql("", ""));
}

TEST(CaseInsensitiveEqualCString, SameStringsAreEqual) {
  const CaseInsensitiveEqual eql;
  EXPECT_TRUE(eql("foobar", "foobar"));
}

TEST(CaseInsensitiveEqualCString, DifferentCasesAreEqual) {
  const CaseInsensitiveEqual eql;
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
  const CaseInsensitiveEqual eql;
  EXPECT_FALSE(eql("foobar", "foobar2"));
  EXPECT_FALSE(eql("foobar2", "foobar"));
}

TEST(CaseInsensitiveEqualCString, DifferentStringsAreNotEqual) {
  const CaseInsensitiveEqual eql;
  EXPECT_FALSE(eql("foobar", "foocar"));
  EXPECT_FALSE(eql("foocar", "foobar"));
}

TEST(CaseInsensitiveEqualStringView, EmptyStringsAreEqual) {
  const CaseInsensitiveEqual eql;
  EXPECT_TRUE(eql("", ""));
}

TEST(CaseInsensitiveEqualStringView, SameStringsAreEqual) {
  const CaseInsensitiveEqual eql;
  EXPECT_TRUE(eql("foobar", "foobar"));
}

TEST(CaseInsensitiveEqualStringView, DifferentCasesAreEqual) {
  const CaseInsensitiveEqual eql;
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
  const CaseInsensitiveEqual eql;
  EXPECT_FALSE(eql("foobar", "foobar2"));
  EXPECT_FALSE(eql("foobar2", "foobar"));
}

TEST(CaseInsensitiveEqualStringView, DifferentStringsAreNotEqual) {
  const CaseInsensitiveEqual eql;
  EXPECT_FALSE(eql("foobar", "foocar"));
  EXPECT_FALSE(eql("foocar", "foobar"));
}

// -------------------------------------------------------------------------- //

TEST(CaseInsensitiveCompare, ComparisonIgnoresCasing) {
  const CaseInsensitiveCompare cmp;
  EXPECT_EQ(cmp("foo", "foo"), 0);
  EXPECT_EQ(cmp("foo", "FOO"), 0);
  EXPECT_EQ(cmp("foo", "fOo"), 0);

  EXPECT_LT(cmp("bar", "foo"), 0);
  EXPECT_LT(cmp("BAR", "foo"), 0);
  EXPECT_LT(cmp("bar", "FOO"), 0);

  EXPECT_GT(cmp("foo", "bar"), 0);
  EXPECT_GT(cmp("FOO", "bar"), 0);
  EXPECT_GT(cmp("foo", "BAR"), 0);
}

TEST(CaseInsensitiveLess, ComparisonIgnoresCasing) {
  const CaseInsensitiveLess less;
  EXPECT_FALSE(less("foo", "foo"));
  EXPECT_FALSE(less("foo", "FOO"));
  EXPECT_FALSE(less("foo", "fOo"));

  EXPECT_TRUE(less("bar", "foo"));
  EXPECT_TRUE(less("BAR", "foo"));
  EXPECT_TRUE(less("bar", "FOO"));

  EXPECT_FALSE(less("foo", "bar"));
  EXPECT_FALSE(less("FOO", "bar"));
  EXPECT_FALSE(less("foo", "BAR"));
}

// -------------------------------------------------------------------------- //

TEST(CaseInsensitiveMapping, HashAndEqualsShouldEnableCaseInsensitiveMaps) {
  std::unordered_map<
    std::string, std::string, CaseInsensitiveHash, CaseInsensitiveEqual
  > test_map;

  test_map["foobar"] = "fizzbang";
  EXPECT_TRUE(test_map.contains("foobar"));
  EXPECT_TRUE(test_map.contains("Foobar"));
  EXPECT_TRUE(test_map.contains("fooBar"));
  EXPECT_TRUE(test_map.contains("FOObar"));

  EXPECT_EQ(test_map["foobar"], "fizzbang");
  EXPECT_EQ(test_map["Foobar"], "fizzbang");
  EXPECT_EQ(test_map["fooBar"], "fizzbang");
  EXPECT_EQ(test_map["FOObar"], "fizzbang");

  test_map["fooBAR"] = "new value";
  EXPECT_EQ(test_map["foobar"], "new value");
  EXPECT_EQ(test_map["Foobar"], "new value");
  EXPECT_EQ(test_map["fooBar"], "new value");
  EXPECT_EQ(test_map["FOObar"], "new value");
}

TEST(CaseInsensitiveMapping, ShouldAllowHetergenousLookup) {
  std::map<std::string, std::string, CaseInsensitiveLess> test_map;

  const char cstr[] = "foobar";
  const std::string str = "foobar";
  const std::string_view view = "foobar";

  test_map["foobar"] = "fizzbang";
  EXPECT_TRUE(test_map.contains(cstr));
  EXPECT_TRUE(test_map.contains(str));
  EXPECT_TRUE(test_map.contains(view));

  EXPECT_EQ(test_map.find(cstr)->second, "fizzbang");
  EXPECT_EQ(test_map.find(str)->second, "fizzbang");
  EXPECT_EQ(test_map.find(view)->second, "fizzbang");

  test_map["fooBAR"] = "new value";
  EXPECT_EQ(test_map.find(cstr)->second, "new value");
  EXPECT_EQ(test_map.find(str)->second, "new value");
  EXPECT_EQ(test_map.find(view)->second, "new value");
}

// -------------------------------------------------------------------------- //

TEST(Concatenate, StringLiterals) {
  EXPECT_EQ(cat("foobar"), "foobar");
  EXPECT_EQ(cat("foo", "bar"), "foobar");
  EXPECT_EQ(cat("f", "oo", "b", "ar"), "foobar");
}

TEST(Concatenate, Strings) {
  EXPECT_EQ(cat(std::string{"foobar"}), "foobar");
  EXPECT_EQ(cat(std::string{"foo"}, std::string{"bar"}), "foobar");
}

TEST(Concatenate, StringViews) {
  EXPECT_EQ(cat(std::string_view{"foobar"}), "foobar");
  EXPECT_EQ(cat(std::string_view{"foo"}, std::string_view{"bar"}), "foobar");
}

TEST(Concatenate, MixedStrings) {
  EXPECT_EQ(
    cat("foo", std::string{"bar"}, std::string_view{"bang"}),
    "foobarbang"
  );
}

TEST(Concatenate, Chars) {
  EXPECT_EQ(cat('f'), "f");
  EXPECT_EQ(cat('f', 'o', 'o'), "foo");
}

TEST(Concatenate, Integers) {
  EXPECT_EQ(cat(1), "1");
  EXPECT_EQ(cat(-2), "-2");
  EXPECT_EQ(cat(6, 9), "69");
}

TEST(Concatenate, IntegerLimits) {
  EXPECT_EQ(
    cat(std::numeric_limits<std::uintmax_t>::max()),
    "18446744073709551615"
  );
  EXPECT_EQ(
    cat(std::numeric_limits<std::intmax_t>::max()),
    "9223372036854775807"
  );
  EXPECT_EQ(
    cat(std::numeric_limits<std::intmax_t>::min()),
    "-9223372036854775808"
  );
}

TEST(Concatenate, MixedValues) {
  EXPECT_EQ(cat(1, "fo", 'o', -2, 'b', "ar"), "1foo-2bar");
}

}
}
