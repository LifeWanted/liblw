
#include <cstdio>
#include <gtest/gtest.h>

#include "lw/event.hpp"
#include "lw/fs.hpp"

namespace lw {
namespace tests {

struct FileTests : public testing::Test {
    event::Loop loop;
    std::string fileName = "/tmp/testFile";

    void TearDown( void ){
        std::remove( fileName.c_str() );
    }
};

TEST_F( FileTests, Open ){
    fs::File file( loop );
    bool started = false;
    bool finished = false;
    bool promiseCalled = false;

    file.open( fileName ).then([&]( event::Promise&& next ){
        EXPECT_TRUE( started );
        EXPECT_FALSE( finished );
        promiseCalled = true;
    });

    started = true;
    loop.run();
    finished = true;

    EXPECT_TRUE( promiseCalled );
}

}
}
