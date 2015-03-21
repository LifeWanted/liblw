
#include <gtest/gtest.h>

#include "lw/event.hpp"

namespace lw {
namespace tests {

struct PromiseRejectionTests : public testing::Test {
    const std::int64_t  code    = 42;
    const std::string   message = "Test error";
    const error::Exception test_error;

    PromiseRejectionTests():
        test_error( code, message )
    {}
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
    }, [&]( const error::Exception& err ){
        EXPECT_TRUE( chained );
        EXPECT_FALSE( rejected );

        EXPECT_EQ( code,    err.error_code()    );
        EXPECT_EQ( message, err.what()          );

        rejected = true;
    });

    // The handler is chained, check that it hasn't been run.
    chained = true;
    EXPECT_FALSE( rejected );

    // Resolve the promise and check that the rejected handler is true
    prom.reject( test_error );
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
    }, [&]( const error::Exception& err ){
        EXPECT_TRUE( chained );
        EXPECT_FALSE( rejected );

        EXPECT_EQ( code,    err.error_code()    );
        EXPECT_EQ( message, err.what()          );

        rejected = true;
    });

    // The handler is chained, check that it hasn't been run.
    chained = true;
    EXPECT_FALSE( rejected );

    // Resolve the promise and check that the rejected handler is true
    prom.reject( test_error );
    EXPECT_TRUE( rejected );
}

// -------------------------------------------------------------------------- //

TEST_F( PromiseRejectionTests, UnhandledRejection ){
    // Create the promise and assign a resolve handler.
    event::Promise<> prom;
    prom.future().then([&](){
        FAIL() << "Entered resolve handler for rejected promise.";
    }).then([&](){
        FAIL() << "Entered resolve handler after rejected promise.";
    });

    // Resolve the promise and check that the rejected handler is true
    EXPECT_THROW( prom.reject( test_error ), error::Exception );
}

}
}
