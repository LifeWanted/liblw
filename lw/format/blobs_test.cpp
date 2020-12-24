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

// -------------------------------------------------------------------------- //

TEST(FormatBase64, ConvertBuffersToHexStrings) {
  Buffer data{10};
  data.copy("fitzgerald", 10);

  auto encoded = base64(data);
  std::string str = std::string(encoded.begin(), encoded.end());
  EXPECT_EQ(str, "Zml0emdlcmFsZA==");
}

TEST(FormatBase64, Streamable) {
  Buffer data{8};
  data.copy("arugulas", 8);

  std::stringstream stream;
  stream << base64(data);
  EXPECT_EQ(stream.str(), "YXJ1Z3VsYXM=");
}

}
}
