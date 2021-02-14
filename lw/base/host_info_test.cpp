#include "lw/base/host_info.h"

#include <cctype>
#include <endian.h>

#include "gtest/gtest.h"

namespace lw {
namespace {

TEST(Endian, BigEndianCheckMatchesSystem) {
  const bool host_is_big_endian = ::htobe32(1) == 1;
  EXPECT_EQ(is_big_endian_byte_order(), host_is_big_endian);
}

TEST(Endian, Flip8BitOrder) {
  EXPECT_EQ(flip_byte_order(static_cast<std::uint8_t>(0x12)), 0x12);
}

TEST(Endian, Flip16BitOrder) {
  EXPECT_EQ(flip_byte_order(static_cast<std::uint16_t>(0x1234)), 0x3412);
}

TEST(Endian, Flip32BitOrder) {
  EXPECT_EQ(
    flip_byte_order(static_cast<std::uint32_t>(0x12345678)),
    0x78563412
  );
}

TEST(Endian, Flip64BitOrder) {
  EXPECT_EQ(
    flip_byte_order(static_cast<std::uint64_t>(0x123456789abcdf01)),
    0x01dfbc9a78563412
  );
}

}
}
