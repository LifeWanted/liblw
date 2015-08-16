
#include <chrono>
#include <cstdio>
#include <gtest/gtest.h>
#include <unistd.h>

#include "lw/event.hpp"
#include "lw/io.hpp"
#include "lw/memory.hpp"

using namespace std::chrono_literals;

namespace lw {
namespace tests {

struct PipeTests : public testing::Test {
    event::Loop loop;
    int pipes[2];
    std::string content_str = "an awesome message to keep";
    memory::Buffer contents;

    PipeTests(void):
        contents(content_str.size())
    {
        contents.copy(content_str.begin(), content_str.end());
    }

    void SetUp(void) override {
        ::pipe(pipes);
    }

    void TearDown(void) override {
        ::close(pipes[0]);
        ::close(pipes[1]);
    }
};

// -------------------------------------------------------------------------- //

TEST_F(PipeTests, Read){
    io::Pipe pipe(loop);
    bool started = false;
    bool finished = false;
    bool promise_called = false;

    // Set up the pipe to read.
    pipe.open(pipes[0]);
    pipe.read([&](const std::shared_ptr<const memory::Buffer>& buffer){
        EXPECT_TRUE(started);
        EXPECT_FALSE(finished);
        EXPECT_EQ(contents, *buffer);
    }).then([&](const std::size_t bytes_read){
        promise_called = true;
        EXPECT_TRUE(started);
        EXPECT_FALSE(finished);
        EXPECT_EQ(contents.size(), bytes_read);
    });

    // Separately, create a timeout that will send the data on the pipe once the loop has started.
    event::wait(loop, 0s).then([&](){
        ::write(pipes[1], content_str.c_str(), content_str.size());
        ::fsync(pipes[1]);
        ::close(pipes[1]);
    });

    started = true;
    loop.run();
    finished = true;

    EXPECT_TRUE(promise_called);
}

// -------------------------------------------------------------------------- //

TEST_F(PipeTests, StopRead){
    io::Pipe pipe(loop);
    bool started = false;
    bool finished = false;
    bool promise_called = false;

    // Set up the pipe to read.
    pipe.open(pipes[0]);
    pipe.read([&](const std::shared_ptr<const memory::Buffer>& buffer){
        EXPECT_TRUE(started);
        EXPECT_FALSE(finished);
        EXPECT_EQ(contents, *buffer);

        pipe.stop_read();
    }).then([&](const std::size_t bytes_read){
        promise_called = true;
        EXPECT_TRUE(started);
        EXPECT_FALSE(finished);
        EXPECT_EQ(contents.size(), bytes_read);
    });

    // Separately, create a timeout that will send the data on the pipe once the loop has started.
    event::wait(loop, 0s).then([&](){
        ::write(pipes[1], content_str.c_str(), content_str.size());
        ::fsync(pipes[1]);
        // No close here, should stop on its own.
    });

    started = true;
    loop.run();
    finished = true;

    EXPECT_TRUE(promise_called);
}

// -------------------------------------------------------------------------- //

TEST_F(PipeTests, Write){
    io::Pipe pipe(loop);
    bool started = false;
    bool finished = false;
    bool promise_called = false;

    std::shared_ptr<memory::Buffer> data(&contents, [](memory::Buffer*){});

    // Set up the pipe to write.
    pipe.open(pipes[1]);
    pipe.write(data).then([&](const std::size_t bytes_written){
        promise_called = true;
        EXPECT_TRUE(started);
        EXPECT_FALSE(finished);
        EXPECT_EQ(contents.size(), bytes_written);
    });

    // Separately, create a timeout that read the data from the pipe once the loop has started.
    event::wait(loop, 0s).then([&](){
        memory::Buffer buffer(1024);
        int bytes_read = ::read(pipes[0], buffer.data(), buffer.capacity());

        EXPECT_EQ((int)contents.size(), bytes_read);
        EXPECT_EQ(contents, memory::Buffer(buffer.data(), bytes_read));
    });

    started = true;
    loop.run();
    finished = true;

    EXPECT_TRUE(promise_called);
}

}
}
