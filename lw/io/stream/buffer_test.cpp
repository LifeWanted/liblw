#include "lw/io/stream/buffer.h"

#include <iostream>

#include "gtest/gtest.h"

namespace lw::io::stream {
namespace {

TEST(StringBufferTest, WorksWithOStream) {
  StringBuffer buffer;
  std::ostream o{&buffer};
  o << "Hello, World!";
  EXPECT_EQ(buffer.string(), "Hello, World!");
}

TEST(StringBufferTest, WorksWithIStream) {
  StringBuffer buffer;
  {
    std::ostream o{&buffer};
    o << "Hello, World!";
  }

  std::istream i{&buffer};
  std::string word;
  i >> word;
  EXPECT_EQ(word, "Hello,");
}

}
}
