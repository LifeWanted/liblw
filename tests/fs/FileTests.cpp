
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

    file.open(
        fileName,
        [&](){
            EXPECT_TRUE( started );
            EXPECT_FALSE( finished );
        }
    );

    started = true;
    loop.run();
    finished = true;
}

}
}
