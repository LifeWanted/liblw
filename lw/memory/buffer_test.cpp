#include "lw/memory/buffer.h"

#include <algorithm>
#include <cstring>
#include <gtest/gtest.h>
#include <string>
#include <tuple>

namespace lw {
namespace {

TEST(Buffer, DefaultConstructor) {
  Buffer buffer;
  EXPECT_EQ(0, buffer.capacity());
  EXPECT_EQ(0, buffer.size());
  EXPECT_EQ(nullptr, buffer.data());
}

TEST(Buffer, WrapConstructor) {
  char data[] = "Hello World!";
  const std::size_t size = sizeof(data);
  Buffer buffer(reinterpret_cast<std::uint8_t*>(data), size);

  // Should adopt the provided character array.
  EXPECT_EQ(size, buffer.size());
  EXPECT_EQ(reinterpret_cast<std::uint8_t*>(data), buffer.data());

  // Should not be a copy of the data.
  buffer[0] = 'B';
  EXPECT_EQ('B', data[0]);
}

TEST(Buffer, MoveConstructor) {
  char data[] = "Hello World!";
  const std::size_t size = sizeof(data);
  Buffer* buffer = new Buffer(reinterpret_cast<std::uint8_t*>(data), size);
  Buffer* buffer2 = new Buffer(std::move(*buffer));

  EXPECT_EQ(size, buffer2->size());
  EXPECT_EQ(reinterpret_cast<std::uint8_t*>(data), buffer2->data());

  EXPECT_NO_THROW({ delete buffer; });

  EXPECT_EQ(size, buffer2->size());
  EXPECT_EQ(reinterpret_cast<std::uint8_t*>(data), buffer2->data());

  EXPECT_NO_THROW({ delete buffer2; });
}

TEST(Buffer, MoveOwnedConstructor) {
  const char* str = "Hello, World!";
  const std::size_t size = std::strlen(str);
  char* data = new char[50];
  std::copy(str, str + size, data );
  Buffer* buffer = new Buffer(reinterpret_cast<std::uint8_t*>(data), size, true);
  Buffer* buffer2 = new Buffer(std::move(*buffer));

  EXPECT_EQ(size, buffer2->size());
  EXPECT_EQ(reinterpret_cast<std::uint8_t*>(data), buffer2->data());

  EXPECT_NO_THROW({ delete buffer; });

  EXPECT_EQ(size, buffer2->size());
  EXPECT_EQ(reinterpret_cast<std::uint8_t*>(data), buffer2->data());

  EXPECT_NO_THROW({ delete buffer2; });
}

TEST(Buffer, SizedAllocationConstructor) {
  Buffer buffer(25);
  EXPECT_EQ(25, buffer.size());
}

TEST(Buffer, IterableCopyConstructor) {
  char data[] = "Hello World!";
  Buffer buffer(std::begin(data), std::end(data));

  // Should be the same size, but different data addresses.
  EXPECT_EQ(sizeof(data), buffer.size());
  EXPECT_NE((void*)data, (void*)buffer.data());

  // Should have copied the contents into itself.
  std::size_t i = 0;
  for (const auto& c : buffer) {
    EXPECT_EQ(data[i], c) << "At position " << i;
    ++i;
  }

  // Should not reflect changes.
  EXPECT_EQ('H', buffer[0]);
  EXPECT_EQ('H', data[0]);
  data[0] = 'B';
  EXPECT_EQ('H', buffer[0]);
  EXPECT_EQ('B', data[0]);
  buffer[0] = 'D';
  EXPECT_EQ('D', buffer[0]);
  EXPECT_EQ('B', data[0]);
}

TEST(Buffer, Destructor) {
  std::uint8_t* data = new std::uint8_t[25];
  Buffer* buffer = new Buffer(data, 25);

  // Should not delete the associated data by default.
  EXPECT_NO_THROW({ delete buffer; });
  EXPECT_NO_THROW({ delete data; });
}

TEST(Buffer, OwnedDataDestructor) {
  std::uint8_t* data = new std::uint8_t[25];
  Buffer* buffer = new Buffer(data, 25, true);

  // Should delete the associated data when told to own it.
  EXPECT_NO_THROW({ delete buffer; });
  // EXPECT_DEATH({ delete data; }, "pointer being freed was not allocated");
}

TEST(Buffer, SetMemory) {
  Buffer buffer(25);
  buffer.set_memory((std::uint8_t)'a');
  for (const auto& c : buffer) {
    EXPECT_EQ((std::uint8_t)'a', c);
  }

  buffer.set_memory((std::uint8_t)'f');
  for (const auto& c : buffer) {
    EXPECT_EQ((std::uint8_t)'f', c);
  }
}

TEST(Buffer, BeginEndCopy) {
  const std::uint8_t* bufferValue =
    reinterpret_cast<const std::uint8_t*>("Hello\0\0\0\0\0");
  char data[] = "Hello World!";
  Buffer buffer(10);
  Buffer buffer2(bufferValue, bufferValue + 10);
  buffer.set_memory(0);

  buffer.copy(
    reinterpret_cast<std::uint8_t*>(data),
    reinterpret_cast<std::uint8_t*>(data) + 5
  );
  EXPECT_EQ(buffer2, buffer);
}

TEST(Buffer, SizedCopy) {
  const std::uint8_t* bufferValue =
    reinterpret_cast<const std::uint8_t*>("Hello\0\0\0\0\0");
  char data[] = "Hello World!";
  Buffer buffer(10);
  Buffer buffer2(bufferValue, bufferValue + 10);
  buffer.set_memory(0);

  buffer.copy(reinterpret_cast<std::uint8_t*>(data), 5);
  EXPECT_EQ(buffer2, buffer);
}

TEST(Buffer, MoveOperator) {
  char data[] = "Hello World!";
  const std::size_t size = sizeof(data);
  Buffer* buffer = new Buffer(reinterpret_cast<std::uint8_t*>(data), size);
  Buffer* buffer2 = new Buffer();
  *buffer2 = std::move(*buffer);

  EXPECT_EQ(size, buffer2->size());
  EXPECT_EQ(reinterpret_cast<std::uint8_t*>(data), buffer2->data());

  EXPECT_NO_THROW({ delete buffer; });

  EXPECT_EQ(size, buffer2->size());
  EXPECT_EQ(reinterpret_cast<std::uint8_t*>(data), buffer2->data());

  EXPECT_NO_THROW({ delete buffer2; });
}

TEST(Buffer, MoveOwnedOperator) {
  const char* str = "Hello, World!";
  const std::size_t size = std::strlen(str);
  char* data = new char[50];
  std::copy(str, str + size, data );
  Buffer* buffer = new Buffer(reinterpret_cast<std::uint8_t*>(data), size, true);
  Buffer* buffer2 = new Buffer();
  *buffer2 = std::move(*buffer);

  EXPECT_EQ(size, buffer2->size());
  EXPECT_EQ(reinterpret_cast<std::uint8_t*>(data), buffer2->data());

  EXPECT_NO_THROW({ delete buffer; });

  EXPECT_EQ(size, buffer2->size());
  EXPECT_EQ(reinterpret_cast<std::uint8_t*>(data), buffer2->data());

  EXPECT_NO_THROW({ delete buffer2; });
}

TEST(Buffer, EqualityOperator) {
  Buffer b1(10);
  Buffer b2(10);
  Buffer b3(10);
  Buffer b4(20);
  Buffer b5(b1.data(), b1.size());

  b1.set_memory('a');
  b2.set_memory('a');
  b3.set_memory('f');
  b4.set_memory('a');

  EXPECT_EQ(b1, b2);
  EXPECT_NE(b1, b3);
  EXPECT_NE(b1, b4);
  EXPECT_EQ(b1, b5);
}

TEST(Buffer, TrimPrefix) {
  char data[] = "Hello World!";
  const std::size_t size = sizeof(data);
  const Buffer buffer(reinterpret_cast<std::uint8_t*>(data), size);

  const Buffer trimmed{buffer.trim_prefix(6)};
  EXPECT_EQ(trimmed.size(), 7);
  EXPECT_EQ(trimmed.front(), 'W');
  EXPECT_EQ(trimmed[5], '!');
}

TEST(Buffer, TrimSuffix) {
  char data[] = "Hello World!";
  const std::size_t size = sizeof(data);
  const Buffer buffer(reinterpret_cast<std::uint8_t*>(data), size);

  const Buffer trimmed{buffer.trim_suffix(8)};
  EXPECT_EQ(trimmed.size(), 5);
  EXPECT_EQ(trimmed.front(), 'H');
  EXPECT_EQ(trimmed[4], 'o');
}

}
}
