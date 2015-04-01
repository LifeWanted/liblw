
#include <chrono>
#include <gtest/gtest.h>

#include "lw/event.hpp"

using namespace std::chrono_literals;

namespace lw {
namespace tests {

struct TimeoutHelperTests : public testing::Test {
    typedef std::chrono::high_resolution_clock clock;
    typedef clock::time_point time_point;

    static const std::chrono::milliseconds short_delay;
    static const std::chrono::milliseconds repeat_interval;
    static const std::chrono::milliseconds max_discrepancy;

    event::Loop loop;
};

const std::chrono::milliseconds TimeoutHelperTests::short_delay     = 25ms;
const std::chrono::milliseconds TimeoutHelperTests::repeat_interval =  5ms;
const std::chrono::milliseconds TimeoutHelperTests::max_discrepancy =  2ms;

// -------------------------------------------------------------------------- //

TEST_F( TimeoutHelperTests, WaitNoDelay ){
    time_point start;
    bool resolved = false;

    event::wait( loop, 0s ).then([&](){
        EXPECT_LT( clock::now() - start, max_discrepancy );

        resolved = true;
    });
    EXPECT_FALSE( resolved );

    start = clock::now();
    loop.run();
    EXPECT_TRUE( resolved );
}

// -------------------------------------------------------------------------- //

TEST_F( TimeoutHelperTests, WaitShortDelay ){
    time_point start;
    bool resolved = false;

    event::wait( loop, short_delay ).then([&](){
        auto time_passed = clock::now() - start;
        EXPECT_LT( time_passed, short_delay + max_discrepancy );
        EXPECT_GT( time_passed, short_delay - max_discrepancy );

        resolved = true;
    });
    EXPECT_FALSE( resolved );

    start = clock::now();
    loop.run();
    EXPECT_TRUE( resolved );
}

// -------------------------------------------------------------------------- //

TEST_F( TimeoutHelperTests, WaitUntilNoDelay ){
    time_point start;
    bool resolved = false;

    event::wait_until( loop, clock::now() ).then([&](){
        EXPECT_LT( clock::now() - start, max_discrepancy );

        resolved = true;
    });
    EXPECT_FALSE( resolved );

    start = clock::now();
    loop.run();
    EXPECT_TRUE( resolved );
}

// -------------------------------------------------------------------------- //

TEST_F( TimeoutHelperTests, WaitUntilShortDelay ){
    time_point start;
    bool resolved = false;

    event::wait_until( loop, clock::now() + short_delay ).then([&](){
        auto time_passed = clock::now() - start;
        EXPECT_LT( time_passed, short_delay + max_discrepancy );
        EXPECT_GT( time_passed, short_delay - max_discrepancy );

        resolved = true;
    });
    EXPECT_FALSE( resolved );

    start = clock::now();
    loop.run();
    EXPECT_TRUE( resolved );
}

}
}
