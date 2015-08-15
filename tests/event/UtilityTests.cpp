
#include <gtest/gtest.h>

#include "lw/error.hpp"
#include "lw/event.hpp"

namespace lw {
namespace tests {

struct UtilityTests : public testing::Test {
    event::Loop loop;
};

// ---------------------------------------------------------------------------------------------- //

TEST_F(UtilityTests, ResolveValue){
    bool resolved = false;

    event::resolve(loop, 1).then([&](int value){
        EXPECT_EQ(1, value);
        EXPECT_FALSE(resolved);
        resolved = true;
    });
    EXPECT_FALSE(resolved);

    loop.run();
    EXPECT_TRUE(resolved);
}

// ---------------------------------------------------------------------------------------------- //

TEST_F(UtilityTests, ResolveVoid){
    bool resolved = false;

    event::resolve(loop).then([&](){
        EXPECT_FALSE(resolved);
        resolved = true;
    });
    EXPECT_FALSE(resolved);

    loop.run();
    EXPECT_TRUE(resolved);
}

// ---------------------------------------------------------------------------------------------- //

TEST_F(UtilityTests, RejectValue){
    bool resolved = false;
    bool rejected = false;

    event::reject<int>(loop, error::Exception(1, "Test error")).then([&](int value){
        resolved = true;
    }, [&](const error::Exception& err){
        EXPECT_FALSE(rejected);
        rejected = true;

        EXPECT_EQ(1, err.error_code());
        EXPECT_EQ((std::string)"Test error", (std::string)err.what());
    });
    EXPECT_FALSE(resolved);
    EXPECT_FALSE(rejected);

    loop.run();
    EXPECT_FALSE(resolved);
    EXPECT_TRUE(rejected);
}

// ---------------------------------------------------------------------------------------------- //

TEST_F(UtilityTests, RejectVoid){
    bool resolved = false;
    bool rejected = false;

    event::reject(loop, error::Exception(1, "Test error")).then([&](){
        resolved = true;
    }, [&](const error::Exception& err){
        EXPECT_FALSE(rejected);
        rejected = true;

        EXPECT_EQ(1, err.error_code());
        EXPECT_EQ((std::string)"Test error", (std::string)err.what());
    });
    EXPECT_FALSE(resolved);
    EXPECT_FALSE(rejected);

    loop.run();
    EXPECT_FALSE(resolved);
    EXPECT_TRUE(rejected);
}

}
}
