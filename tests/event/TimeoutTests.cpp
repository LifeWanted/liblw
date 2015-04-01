
#include <chrono>
#include <gtest/gtest.h>

#include "lw/event.hpp"

using namespace std::chrono_literals;

namespace lw {
namespace tests {

struct TimeoutTests : public testing::Test {
    typedef std::chrono::high_resolution_clock clock;
    typedef clock::time_point time_point;

    static const std::chrono::milliseconds short_delay;
    static const std::chrono::milliseconds repeat_interval;
    static const std::chrono::milliseconds max_discrepancy;

    event::Loop loop;
};

const std::chrono::milliseconds TimeoutTests::short_delay       = 25ms;
const std::chrono::milliseconds TimeoutTests::repeat_interval   =  5ms;
const std::chrono::milliseconds TimeoutTests::max_discrepancy   =  2ms;

// -------------------------------------------------------------------------- //

TEST_F( TimeoutTests, NoDelay ){
    time_point start;
    bool resolved = false;

    event::Timeout timeout( loop );
    timeout.start( 0s ).then([&](){
        EXPECT_LT( clock::now() - start, max_discrepancy );

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
    timeout.start( short_delay ).then([&](){
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

TEST_F( TimeoutTests, Repeat ){
    time_point start;
    time_point previous_call;
    int call_count = 0;

    event::Timeout timeout( loop );
    timeout.repeat( repeat_interval, [&]( event::Timeout& repeat_timeout ){
        ++call_count;
        time_point call_time = clock::now();

        { // Check the total delay since starting.
            auto time_passed = call_time - start;
            EXPECT_LT(
                time_passed,
                (repeat_interval + max_discrepancy) * call_count
            ) << "call_count: " << call_count;
            EXPECT_GT(
                time_passed,
                (repeat_interval - max_discrepancy) * call_count
            ) << "call_count: " << call_count;
        }

        // Starting with the second call, start comparing the delay.
        if( call_count > 1 ){
            auto time_passed = call_time - previous_call;
            EXPECT_LT(
                time_passed,
                repeat_interval + max_discrepancy
            ) << "call_count: " << call_count;
            EXPECT_GT(
                time_passed,
                repeat_interval - max_discrepancy
            ) << "call_count: " << call_count;
        }

        previous_call = call_time;

        // Stop repeating after 4 calls.
        ASSERT_LT( call_count, 5 );
        if( call_count == 4 ){
            repeat_timeout.stop();
        }
    });
    EXPECT_EQ( 0, call_count );

    start = clock::now();
    loop.run();
    EXPECT_EQ( 4, call_count );
}

}
}
