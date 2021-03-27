#include "lw/io/co/co.h"

#include "gtest/gtest.h"
#include "lw/co/scheduler.h"
#include "lw/co/task.h"
#include "lw/io/co/testing/string_readable.h"

namespace lw::io {
namespace {

using ::lw::io::testing::StringReadable;

TEST(CoReader, IsGood) {
  co::Scheduler::this_thread().schedule([]() -> co::Task {
    StringReadable readable{"foobar"};
    CoReader<StringReadable> reader{readable};
    EXPECT_TRUE(reader.good());
    auto b = co_await reader.read_until('f', 1000);
    EXPECT_TRUE(reader.good());
    EXPECT_FALSE(reader.eof());

    StringReadable empty_readable{""};
    CoReader<StringReadable> empty_reader{empty_readable};
    EXPECT_FALSE(empty_reader.good());
    EXPECT_TRUE(empty_reader.eof());
  });
  co::Scheduler::this_thread().run();
}

TEST(CoReader, ReadBytes) {
  co::Scheduler::this_thread().schedule([]() -> co::Task {
    StringReadable readable{"foobar"};
    CoReader<StringReadable> reader{readable};
    Buffer b = co_await reader.read(3);
    EXPECT_EQ(static_cast<std::string_view>(b), "foo");
    EXPECT_TRUE(reader.good());
    EXPECT_FALSE(reader.eof());

    b = co_await reader.read(100);
    EXPECT_EQ(static_cast<std::string_view>(b), "bar");
    EXPECT_FALSE(reader.good());
    EXPECT_TRUE(reader.eof());
  });
  co::Scheduler::this_thread().run();
}

TEST(CoReader, ReadUntilChar) {
  co::Scheduler::this_thread().schedule([]() -> co::Task {
    StringReadable readable{"foobar"};
    CoReader<StringReadable> reader{readable};
    Buffer b = co_await reader.read_until('b');
    EXPECT_EQ(static_cast<std::string_view>(b), "foob");
    EXPECT_TRUE(reader.good());
    EXPECT_FALSE(reader.eof());
  });
  co::Scheduler::this_thread().run();
}

TEST(CoReader, ReadUntilString) {
  co::Scheduler::this_thread().schedule([]() -> co::Task {
    StringReadable readable{"foobar"};
    CoReader<StringReadable> reader{readable};
    Buffer b = co_await reader.read_until("bar");
    EXPECT_EQ(static_cast<std::string_view>(b), "foobar");
    EXPECT_FALSE(reader.good());
    EXPECT_TRUE(reader.eof());
  });
  co::Scheduler::this_thread().run();
}

}
}
