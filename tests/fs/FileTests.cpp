
#include <cstdio>
#include <fstream>
#include <gtest/gtest.h>

#include "lw/event.hpp"
#include "lw/fs.hpp"

namespace lw {
namespace tests {

struct FileTests : public testing::Test {
    event::Loop loop;
    std::string fileName = "/tmp/liblw-filetests-testfile";
    std::string contents = "an awesome message to keep";

    void TearDown( void ){
        std::remove( fileName.c_str() );
    }
};

// -------------------------------------------------------------------------- //

TEST_F( FileTests, Open ){
    fs::File file( loop );
    bool started = false;
    bool finished = false;
    bool promiseCalled = false;

    file.open( fileName ).then< void >([&]( event::Promise<>&& next ){
        EXPECT_TRUE( started );
        EXPECT_FALSE( finished );
        promiseCalled = true;
    });

    started = true;
    loop.run();
    finished = true;

    EXPECT_TRUE( promiseCalled );
}

// -------------------------------------------------------------------------- //

TEST_F( FileTests, Close ){
    fs::File file( loop );

    file.open( fileName ).then([&](){
        return file.close();
    });

    loop.run();
}

// -------------------------------------------------------------------------- //

TEST_F( FileTests, Write ){
    fs::File file( loop );

    file.open( fileName ).then([&](){
        return file.write( contents );
    }).then([&](){
        return file.close();
    });

    loop.run();

    std::ifstream testStream( fileName );
    std::string testString;
    std::getline( testStream, testString );

    EXPECT_EQ( testString, contents );
}

}
}
