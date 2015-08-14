
#include <chrono>
#include <gtest/gtest.h>

#include "lw/event.hpp"

using namespace std::chrono;
using namespace std::chrono_literals;

namespace lw {
namespace tests {

struct TimeoutHelperTests : public testing::Test {
    typedef high_resolution_clock clock;
    typedef clock::time_point time_point;

    static const milliseconds short_delay;
    static const milliseconds repeat_interval;
    static const milliseconds max_discrepancy;

    event::Loop loop;
};

const milliseconds TimeoutHelperTests::short_delay      = 25ms;
const milliseconds TimeoutHelperTests::repeat_interval  =  5ms;
const milliseconds TimeoutHelperTests::max_discrepancy  =  3ms;

// ---------------------------------------------------------------------------------------------- //

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

// ---------------------------------------------------------------------------------------------- //

TEST_F(TimeoutHelperTests, WaitShortDelay){
    time_point start;
    bool resolved = false;

    event::wait(loop, short_delay).then([&](){
        auto time_passed = duration_cast<milliseconds>(clock::now() - start);
        EXPECT_LT(time_passed, short_delay + max_discrepancy);
        EXPECT_GT(time_passed, short_delay - max_discrepancy);

        resolved = true;
    });
    EXPECT_FALSE(resolved);

    start = clock::now();
    loop.run();
    EXPECT_TRUE(resolved);
}

// ---------------------------------------------------------------------------------------------- //

TEST_F(TimeoutHelperTests, WaitUntilNoDelay){
    time_point start;
    bool resolved = false;

    event::wait_until(loop, clock::now()).then([&](){
        EXPECT_LT(clock::now() - start, max_discrepancy);

        resolved = true;
    });
    EXPECT_FALSE(resolved);

    start = clock::now();
    loop.run();
    EXPECT_TRUE(resolved);
}

// ---------------------------------------------------------------------------------------------- //

TEST_F(TimeoutHelperTests, WaitUntilShortDelay){
    time_point start;
    bool resolved = false;

    event::wait_until(loop, clock::now() + short_delay).then([&](){
        auto time_passed = duration_cast<milliseconds>(clock::now() - start);
        EXPECT_LT(time_passed, short_delay + max_discrepancy);
        EXPECT_GT(time_passed, short_delay - max_discrepancy);

        resolved = true;
    });
    EXPECT_FALSE(resolved);

    start = clock::now();
    loop.run();
    EXPECT_TRUE(resolved);
}

// ---------------------------------------------------------------------------------------------- //

TEST_F( TimeoutHelperTests, Repeat ){
    time_point start;
    time_point previous_call;
    int call_count = 0;
    bool resolved = false;

    event::repeat( loop, repeat_interval, [&]( event::Timeout& timeout ){
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
            timeout.stop();
        }
    }).then([&](){
        EXPECT_FALSE( resolved );
        resolved = true;
    });
    EXPECT_EQ( 0, call_count );
    EXPECT_FALSE( resolved );

    start = clock::now();
    loop.run();
    EXPECT_EQ( 4, call_count );
    EXPECT_TRUE( resolved );
}

}
}
