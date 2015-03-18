
#include <gtest/gtest.h>

#include "lw/event.hpp"

namespace lw {
namespace tests {

struct PromiseVoidSynchronousTests : public testing::Test {
    event::Loop loop;
};

// -------------------------------------------------------------------------- //

TEST_F( PromiseVoidSynchronousTests, Void_PromiseInt ){
    // State observation variables.
    bool chained    = false;
    bool resolved   = false;
    bool resolved2  = false;

    const int value = 42;

    // Create the promise and assign a resolve handler.
    event::Promise<> prom;
    prom.future().then< int >([&]( event::Promise< int >&& prom2 ){
        EXPECT_TRUE( chained );
        EXPECT_FALSE( resolved );
        EXPECT_FALSE( resolved2 );

        resolved = true;
        prom2.resolve( value );
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

TEST_F( PromiseVoidSynchronousTests, Void_PromiseVoid ){
    // State observation variables.
    bool chained    = false;
    bool resolved   = false;
    bool resolved2  = false;

    // Create the promise and assign a resolve handler.
    event::Promise<> prom;
    prom.future().then([&]( event::Promise<>&& prom2 ){
        EXPECT_TRUE( chained );
        EXPECT_FALSE( resolved );
        EXPECT_FALSE( resolved2 );

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
    prom.resolve();
    EXPECT_TRUE( resolved );
    EXPECT_TRUE( resolved2 );
}

// -------------------------------------------------------------------------- //

TEST_F( PromiseVoidSynchronousTests, FutureInt_Void ){
    // State observation variables.
    bool chained    = false;
    bool resolved   = false;
    bool resolved2  = false;

    const int value = 42;

    // Create the promise and assign a resolve handler.
    event::Promise<> prom;
    event::Promise< int > prom2;
    prom.future().then([&](){
        EXPECT_TRUE( chained );
        EXPECT_FALSE( resolved );
        EXPECT_FALSE( resolved2 );

        resolved = true;
        return prom2.future();
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
    EXPECT_FALSE( resolved2 );

    // Now resolve the 2nd promise.
    prom2.resolve( value );
    EXPECT_TRUE( resolved );
    EXPECT_TRUE( resolved2 );
}

// -------------------------------------------------------------------------- //

TEST_F( PromiseVoidSynchronousTests, Int_Void ){
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

TEST_F( PromiseVoidSynchronousTests, Void_Void ){
    // State observation variables.
    bool chained    = false;
    bool resolved   = false;
    bool resolved2  = false;

    // Create the promise and assign a resolve handler.
    event::Promise<> prom;
    prom.future().then([&](){
        EXPECT_TRUE( chained );
        EXPECT_FALSE( resolved );
        EXPECT_FALSE( resolved2 );

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
    prom.resolve();
    EXPECT_TRUE( resolved );
    EXPECT_TRUE( resolved2 );
}

// -------------------------------------------------------------------------- //

TEST_F( PromiseVoidSynchronousTests, ConnectPromise ){
    // State observation variables.
    bool chained    = false;
    bool resolved   = false;

    // Create the promise and assign a resolve handler.
    event::Promise<> prom;
    event::Promise<> prom2;
    prom2.future().then([&](){
        EXPECT_TRUE( chained );
        EXPECT_FALSE( resolved );

        resolved = true;
    });
    prom.future().then( std::move( prom2 ) );

    // The handler is chained, check that it hasn't been run.
    chained = true;
    EXPECT_FALSE( resolved );

    // Resolve the promise and check that the resolved handler is true
    prom.resolve();
    EXPECT_TRUE( resolved );
}

}
}
