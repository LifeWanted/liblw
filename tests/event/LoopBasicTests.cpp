
#include <gtest/gtest.h>

#include "lw/event.hpp"

namespace lw {
namespace tests {

struct LoopBasicTests : public testing::Test {
    event::Loop loop;
};

TEST_F( LoopBasicTests, HelloWorld ){
    loop.run();
}

TEST_F( LoopBasicTests, IdleLoop ){
    const std::uint64_t ticks = 10000;
    std::uint64_t counter = 0;
    event::Idle idle( loop );

    idle.start([&](){
        if( ++counter >= ticks ){
            idle.stop();
        }
    });

    EXPECT_EQ( 0, counter );

    loop.run();

    EXPECT_EQ( ticks, counter );
}

}
}
