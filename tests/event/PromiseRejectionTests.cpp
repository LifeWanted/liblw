
#include <gtest/gtest.h>

#include "lw/event.hpp"

namespace lw {
namespace tests {

struct PromiseRejectionTests : public testing::Test {
};

// -------------------------------------------------------------------------- //

TEST_F( PromiseRejectionTests, Reject ){
    // State observation variables.
    bool chained    = false;
    bool rejected   = false;

    // Create the promise and assign a resolve handler.
    event::Promise<> prom;
    prom.future().then([&](){
        FAIL() << "Entered resolve handler for rejected promise.";
    },[&](){
        EXPECT_TRUE( chained );
        EXPECT_FALSE( rejected );

        rejected = true;
    });

    // The handler is chained, check that it hasn't been run.
    chained = true;
    EXPECT_FALSE( rejected );

    // Resolve the promise and check that the rejected handler is true
    prom.reject();
    EXPECT_TRUE( rejected );
}

// -------------------------------------------------------------------------- //

TEST_F( PromiseRejectionTests, RejectBubbling ){
    // State observation variables.
    bool chained    = false;
    bool rejected   = false;

    // Create the promise and assign a resolve handler.
    event::Promise<> prom;
    prom.future().then([&](){
        FAIL() << "Entered resolve handler for rejected promise.";
    }).then([&](){
        FAIL() << "Entered resolve handler after rejected promise.";
    }, [&](){
        EXPECT_TRUE( chained );
        EXPECT_FALSE( rejected );

        rejected = true;
    });

    // The handler is chained, check that it hasn't been run.
    chained = true;
    EXPECT_FALSE( rejected );

    // Resolve the promise and check that the rejected handler is true
    prom.reject();
    EXPECT_TRUE( rejected );
}

}
}
