
#include <gtest/gtest.h>

#include "lw/event.hpp"

namespace lw {
namespace tests {

struct PromiseSynchronousVoidTests : public testing::Test {
    event::Loop loop;
};

// -------------------------------------------------------------------------- //

TEST_F( PromiseSynchronousVoidTests, IntVoidTest ){
    // State observation variables.
    bool chained    = false;
    bool resolved   = false;
    bool resolved2  = false;

    const int value = 42;

    // Create the promise and assign a resolve handler.
    event::Promise<> prom;
    prom.future().then([&](){
        EXPECT_TRUE( chained );
        EXPECT_FALSE( resolved );
        EXPECT_FALSE( resolved2 );

        resolved = true;
        return value;
    }).then([&]( int result ){
        EXPECT_TRUE( chained );
        EXPECT_TRUE( resolved );
        EXPECT_FALSE( resolved2 );
        EXPECT_EQ( value, result );

        resolved2 = true;
    });

    // The handler is chained, check that it hasn't been run.
    chained = true;
    EXPECT_FALSE( resolved );
    EXPECT_FALSE( resolved2 );

    // Resolve the promise and check that the resolved handler is true
    prom.resolve();
    EXPECT_TRUE( resolved );
    EXPECT_TRUE( resolved2 );
}

// -------------------------------------------------------------------------- //

TEST_F( PromiseSynchronousVoidTests, VoidVoidTest ){
    // State observation variables.
    bool chained    = false;
    bool resolved   = false;

    // Create the promise and assign a resolve handler.
    event::Promise<> prom;
    prom.future().then([&](){
        EXPECT_TRUE( chained );
        EXPECT_FALSE( resolved );

        resolved = true;
    });

    // The handler is chained, check that it hasn't been run.
    chained = true;
    EXPECT_FALSE( resolved );

    // Resolve the promise and check that the resolved handler is true
    prom.resolve();
    EXPECT_TRUE( resolved );
}

}
}
