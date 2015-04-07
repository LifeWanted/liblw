
#include <cstdint>
#include <gtest/gtest.h>

#include "lw/event.hpp"

namespace lw {
namespace tests {

struct Destructor {
    static std::uint64_t construct_count;
    static std::uint64_t default_construct_count;
    static std::uint64_t move_construct_count;
    static std::uint64_t copy_construct_count;
    static std::uint64_t destruct_count;

    // ---------------------------------------------------------------------- //

    Destructor( void ){
        ++construct_count;
        ++default_construct_count;
    }

    // ---------------------------------------------------------------------- //

    Destructor( const Destructor& ){
        ++construct_count;
        ++copy_construct_count;
    }

    // ---------------------------------------------------------------------- //

    Destructor( Destructor&& ){
        ++construct_count;
        ++move_construct_count;
    }

    // ---------------------------------------------------------------------- //

    ~Destructor( void ){
        ++destruct_count;
    }
};
std::uint64_t Destructor::construct_count           = 0;
std::uint64_t Destructor::default_construct_count   = 0;
std::uint64_t Destructor::move_construct_count      = 0;
std::uint64_t Destructor::copy_construct_count      = 0;
std::uint64_t Destructor::destruct_count            = 0;

// -------------------------------------------------------------------------- //

struct PromiseDestructionTests : public testing::Test {
};

// -------------------------------------------------------------------------- //

TEST_F( PromiseDestructionTests, Basic ){
    auto promise = std::make_shared< event::Promise<> >();

    {
        Destructor monitor;
        ASSERT_EQ( 1, Destructor::construct_count );
        promise->future().then([ this, monitor ](){
            EXPECT_LE( 2, Destructor::construct_count       );
            EXPECT_LE( 1, Destructor::copy_construct_count  );
            EXPECT_LE( 1, Destructor::destruct_count        );
        });
    }
    EXPECT_LE( 2, Destructor::construct_count   );
    EXPECT_LE( 1, Destructor::destruct_count    );

    promise->resolve();
    promise.reset();
    EXPECT_EQ( Destructor::construct_count, Destructor::destruct_count );
}

}
}
