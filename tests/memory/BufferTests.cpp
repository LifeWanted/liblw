
#include <algorithm>
#include <gtest/gtest.h>
#include <string>
#include <tuple>

#include "lw/memory.hpp"

using namespace std;

namespace lw {
namespace tests {

struct BufferTests : public testing::Test {};

// ---------------------------------------------------------------------------------------------- //

TEST_F(BufferTests, DefaultConstructor){
    memory::Buffer buffer;
    EXPECT_EQ(0, buffer.capacity());
    EXPECT_EQ(0, buffer.size());
    EXPECT_EQ(nullptr, buffer.data());
}

// ---------------------------------------------------------------------------------------------- //

TEST_F(BufferTests, WrapConstructor){
    char data[] = {'H', 'e', 'l', 'l', 'o', ',', ' ', 'W', 'o', 'r', 'l', 'd', '!', '\0'};
    const std::size_t size = sizeof(data);
    memory::Buffer buffer((memory::byte*)data, size);

    // Should adopt the provided character array.
    EXPECT_EQ(size, buffer.size());
    EXPECT_EQ((memory::byte*)data, buffer.data());

    // Should not be a copy of the data.
    buffer[0] = 'B';
    EXPECT_EQ('B', data[0]);
}

// ---------------------------------------------------------------------------------------------- //

TEST_F(BufferTests, MoveConstructor){
    char data[] = {'H', 'e', 'l', 'l', 'o', ',', ' ', 'W', 'o', 'r', 'l', 'd', '!', '\0'};
    const std::size_t size = sizeof(data);
    memory::Buffer *buffer = new memory::Buffer((memory::byte*)data, size);
    memory::Buffer *buffer2 = new memory::Buffer(std::move(*buffer));

    EXPECT_EQ(size, buffer2->size());
    EXPECT_EQ((memory::byte*)data, buffer2->data());

    EXPECT_NO_THROW({ delete buffer; });

    EXPECT_EQ(size, buffer2->size());
    EXPECT_EQ((memory::byte*)data, buffer2->data());

    EXPECT_NO_THROW({ delete buffer2; });
}

// ---------------------------------------------------------------------------------------------- //

TEST_F(BufferTests, MoveOwnedConstructor){
    const char* str = "Hello, World!";
    const std::size_t size = std::strlen(str);
    char* data = new char[50];
    std::copy(str, str + size, data );
    memory::Buffer *buffer = new memory::Buffer((memory::byte*)data, size, true); // Own it!
    memory::Buffer *buffer2 = new memory::Buffer(std::move(*buffer));

    EXPECT_EQ(size, buffer2->size());
    EXPECT_EQ((memory::byte*)data, buffer2->data());

    EXPECT_NO_THROW({ delete buffer; });

    EXPECT_EQ(size, buffer2->size());
    EXPECT_EQ((memory::byte*)data, buffer2->data());

    EXPECT_NO_THROW({ delete buffer2; });
}

}
}
