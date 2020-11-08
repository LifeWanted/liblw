#include "lw/format/blobs.h"

#include <sstream>
#include <string>

#include "gtest/gtest.h"

namespace lw::format {
namespace {

TEST(FormatHex, ConvertBuffersToHexStrings) {
  Buffer data{3};
  data.copy("foo", 3);

  auto hexed = hex(data);
  std::string str = std::string(hexed.begin(), hexed.end());
  EXPECT_EQ(str, "666f6f");
}

TEST(FormatHex, Streamable) {
  Buffer data{3};
  data.copy("bar", 3);

  std::stringstream stream;
  stream << hex(data);
  EXPECT_EQ(stream.str(), "626172");
}

}
}
