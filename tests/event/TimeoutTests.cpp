
#include <chrono>
#include <gtest/gtest.h>

#include "lw/event.hpp"

using namespace std::chrono_literals;

namespace lw {
namespace tests {

struct TimeoutTests : public testing::Test {
    typedef std::chrono::high_resolution_clock clock;
    typedef clock::time_point time_point;

    event::Loop loop;
};

// -------------------------------------------------------------------------- //

TEST_F( TimeoutTests, NoDelay ){
    time_point start;
    bool resolved = false;

    event::Timeout timeout( loop );
    timeout.start( 0s ).then([&](){
        EXPECT_LT( clock::now() - start, 100000ns );

        resolved = true;
    });
    EXPECT_FALSE( resolved );

    start = clock::now();
    loop.run();
    EXPECT_TRUE( resolved );
}

// -------------------------------------------------------------------------- //

TEST_F( TimeoutTests, ShortDelay ){
    time_point start;
    bool resolved = false;

    event::Timeout timeout( loop );
    timeout.start( 50ms ).then([&](){
        auto time_passed = clock::now() - start;
        EXPECT_LT( time_passed, 60ms );
        EXPECT_GT( time_passed, 40ms );

        resolved = true;
    });
    EXPECT_FALSE( resolved );

    start = clock::now();
    loop.run();
    EXPECT_TRUE( resolved );
}

}
}
