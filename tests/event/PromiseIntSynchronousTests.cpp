
#include <gtest/gtest.h>

#include "lw/event.hpp"

namespace lw {
namespace tests {

struct PromiseIntSynchronousTests : public testing::Test {
    event::Loop loop;
};

// -------------------------------------------------------------------------- //

TEST_F( PromiseIntSynchronousTests, Void_Int_PromiseInt ){
    // State observation variables.
    bool chained    = false;
    bool resolved   = false;
    bool resolved2  = false;

    const int value     = 42;
    const int value2    = 47;

    // Create the promise and assign a resolve handler.
    event::Promise< int > prom;
    prom.future().then< int >([&]( int result, event::Promise< int >&& prom2 ){
        EXPECT_TRUE( chained );
        EXPECT_FALSE( resolved );
        EXPECT_FALSE( resolved2 );
        EXPECT_EQ( value, result );

        resolved = true;
        prom2.resolve( value2 );
    }).then([&]( int result ){
        EXPECT_TRUE( chained );
        EXPECT_TRUE( resolved );
        EXPECT_FALSE( resolved2 );
        EXPECT_EQ( value2, result );

        resolved2 = true;
    });

    // The handler is chained, check that it hasn't been run.
    chained = true;
    EXPECT_FALSE( resolved );
    EXPECT_FALSE( resolved2 );

    // Resolve the promise and check that the resolved handler is true
    prom.resolve( value );
    EXPECT_TRUE( resolved );
    EXPECT_TRUE( resolved2 );
}

// -------------------------------------------------------------------------- //

TEST_F( PromiseIntSynchronousTests, Void_Int_PromiseVoid ){
    // State observation variables.
    bool chained    = false;
    bool resolved   = false;
    bool resolved2  = false;

    const int value = 42;

    // Create the promise and assign a resolve handler.
    event::Promise< int > prom;
    prom.future().then([&]( int result, event::Promise<>&& prom2 ){
        EXPECT_TRUE( chained );
        EXPECT_FALSE( resolved );
        EXPECT_FALSE( resolved2 );
        EXPECT_EQ( value, result );

        resolved = true;
        prom2.resolve();
    }).then([&](){
        EXPECT_TRUE( chained );
        EXPECT_TRUE( resolved );
        EXPECT_FALSE( resolved2 );

        resolved2 = true;
    });

    // The handler is chained, check that it hasn't been run.
    chained = true;
    EXPECT_FALSE( resolved );
    EXPECT_FALSE( resolved2 );

    // Resolve the promise and check that the resolved handler is true
    prom.resolve( value );
    EXPECT_TRUE( resolved );
    EXPECT_TRUE( resolved2 );
}

// -------------------------------------------------------------------------- //

TEST_F( PromiseIntSynchronousTests, FutureInt_Int ){
    // State observation variables.
    bool chained    = false;
    bool resolved   = false;
    bool resolved2  = false;

    const int value     = 42;
    const int value2    = 47;

    // Create the promise and assign a resolve handler.
    event::Promise< int > prom;
    event::Promise< int > prom2;
    prom.future().then([&]( int result ){
        EXPECT_TRUE( chained );
        EXPECT_FALSE( resolved );
        EXPECT_FALSE( resolved2 );
        EXPECT_EQ( value, result );

        resolved = true;
        return prom2.future();
    }).then([&]( int result ){
        EXPECT_TRUE( chained );
        EXPECT_TRUE( resolved );
        EXPECT_FALSE( resolved2 );
        EXPECT_EQ( value2, result );

        resolved2 = true;
    });

    // The handler is chained, check that it hasn't been run.
    chained = true;
    EXPECT_FALSE( resolved );
    EXPECT_FALSE( resolved2 );

    // Resolve the promise and check that the resolved handler is true
    prom.resolve( value );
    EXPECT_TRUE( resolved );
    EXPECT_FALSE( resolved2 );

    // Now resolve the 2nd promise.
    prom2.resolve( value2 );
    EXPECT_TRUE( resolved );
    EXPECT_TRUE( resolved2 );
}

// -------------------------------------------------------------------------- //

TEST_F( PromiseIntSynchronousTests, Int_Int ){
    // State observation variables.
    bool chained    = false;
    bool resolved   = false;
    bool resolved2  = false;

    const int value     = 42;
    const int value2    = 47;

    // Create the promise and assign a resolve handler.
    event::Promise< int > prom;
    prom.future().then([&]( int result ){
        EXPECT_TRUE( chained );
        EXPECT_FALSE( resolved );
        EXPECT_FALSE( resolved2 );
        EXPECT_EQ( value, result );

        resolved = true;
        return value2;
    }).then([&]( int result ){
        EXPECT_TRUE( chained );
        EXPECT_TRUE( resolved );
        EXPECT_FALSE( resolved2 );
        EXPECT_EQ( value2, result );

        resolved2 = true;
    });

    // The handler is chained, check that it hasn't been run.
    chained = true;
    EXPECT_FALSE( resolved );
    EXPECT_FALSE( resolved2 );

    // Resolve the promise and check that the resolved handler is true
    prom.resolve( value );
    EXPECT_TRUE( resolved );
    EXPECT_TRUE( resolved2 );
}

// -------------------------------------------------------------------------- //

TEST_F( PromiseIntSynchronousTests, Void_Int ){
    // State observation variables.
    bool chained    = false;
    bool resolved   = false;
    bool resolved2  = false;

    const int value = 42;

    // Create the promise and assign a resolve handler.
    event::Promise< int > prom;
    prom.future().then([&]( int result ){
        EXPECT_TRUE( chained );
        EXPECT_FALSE( resolved );
        EXPECT_FALSE( resolved2 );
        EXPECT_EQ( value, result );

        resolved = true;
    }).then([&](){
        EXPECT_TRUE( chained );
        EXPECT_TRUE( resolved );
        EXPECT_FALSE( resolved2 );

        resolved2 = true;
    });

    // The handler is chained, check that it hasn't been run.
    chained = true;
    EXPECT_FALSE( resolved );
    EXPECT_FALSE( resolved2 );

    // Resolve the promise and check that the resolved handler is true
    prom.resolve( value );
    EXPECT_TRUE( resolved );
    EXPECT_TRUE( resolved2 );
}

// -------------------------------------------------------------------------- //

TEST_F( PromiseIntSynchronousTests, ConnectPromise ){
    // State observation variables.
    bool chained    = false;
    bool resolved   = false;

    const int value = 42;

    // Create the promise and assign a resolve handler.
    event::Promise< int > prom;
    event::Promise< int > prom2;
    prom2.future().then([&]( int result ){
        EXPECT_TRUE( chained );
        EXPECT_FALSE( resolved );
        EXPECT_EQ( value, result );

        resolved = true;
    });
    prom.future().then( std::move( prom2 ) );

    // The handler is chained, check that it hasn't been run.
    chained = true;
    EXPECT_FALSE( resolved );

    // Resolve the promise and check that the resolved handler is true
    prom.resolve( value );
    EXPECT_TRUE( resolved );
}

}
}
