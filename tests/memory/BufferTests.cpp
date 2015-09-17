
#include <algorithm>
#include <exception>
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

// ---------------------------------------------------------------------------------------------- //

TEST_F(BufferTests, MoveResizeConstructor){
    char data[] = {'H', 'e', 'l', 'l', 'o', ',', ' ', 'W', 'o', 'r', 'l', 'd', '!', '\0'};
    const std::size_t size = sizeof(data);
    memory::Buffer *buffer = new memory::Buffer((memory::byte*)data, size);
    memory::Buffer *buffer2 = new memory::Buffer(std::move(*buffer), 5);

    EXPECT_EQ(5, buffer2->size());
    EXPECT_EQ((memory::byte*)data, buffer2->data());

    EXPECT_NO_THROW({ delete buffer; });

    EXPECT_EQ(5, buffer2->size());
    EXPECT_EQ((memory::byte*)data, buffer2->data());

    EXPECT_NO_THROW({ delete buffer2; });
}

// ---------------------------------------------------------------------------------------------- //

TEST_F(BufferTests, SizedAllocationConstructor){
    memory::Buffer buffer(25);
    EXPECT_EQ(25, buffer.size());
}

// ---------------------------------------------------------------------------------------------- //

TEST_F(BufferTests, IterableCopyConstructor){
    char data[] = {'H', 'e', 'l', 'l', 'o', ',', ' ', 'W', 'o', 'r', 'l', 'd', '!', '\0'};
    memory::Buffer buffer(std::begin(data), std::end(data));

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

// ---------------------------------------------------------------------------------------------- //

TEST_F(BufferTests, Destructor){
    memory::byte* data = new memory::byte[25];
    memory::Buffer* buffer = new memory::Buffer(data, 25);

    // Should not delete the associated data by default.
    EXPECT_NO_THROW({ delete buffer; });
    EXPECT_NO_THROW({ delete data; });
}

// ---------------------------------------------------------------------------------------------- //

TEST_F(BufferTests, OwnedDataDestructor){
    memory::byte* data = new memory::byte[25];
    memory::Buffer* buffer = new memory::Buffer(data, 25, true);

    // Should delete the associated data when told to own it.
    EXPECT_NO_THROW({ delete buffer; });
    EXPECT_DEATH({ delete data; }, "pointer being freed was not allocated");
}

}
}
