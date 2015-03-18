
#include <gtest/gtest.h>

#include "lw/event.hpp"

namespace lw {
namespace tests {

struct PromiseSynchronousVoidTests : public testing::Test {
    event::Loop loop;
};

// -------------------------------------------------------------------------- //

TEST_F( PromiseSynchronousVoidTests, IntVoidTest ){
    
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
