
#include <cstdio>
#include <fstream>
#include <gtest/gtest.h>

#include "lw/event.hpp"
#include "lw/io.hpp"
#include "lw/memory.hpp"

namespace lw {
namespace tests {

struct FileTests : public testing::Test {
    event::Loop loop;
    std::string file_name   = "/tmp/liblw-filetests-testfile";
    std::string content_str = "an awesome message to keep";
    memory::Buffer contents;

    FileTests( void ):
        contents( content_str.size() )
    {
        contents.copy( content_str.begin(), content_str.end() );
    }

    void TearDown( void ){
        std::remove( file_name.c_str() );
    }
};

// -------------------------------------------------------------------------- //

TEST_F( FileTests, Open ){
    io::File file( loop );
    bool started = false;
    bool finished = false;
    bool promise_called = false;

    file.open( file_name ).then([&](){
        EXPECT_TRUE( started );
        EXPECT_FALSE( finished );
        promise_called = true;
    });

    started = true;
    loop.run();
    finished = true;

    EXPECT_TRUE( promise_called );
}

// -------------------------------------------------------------------------- //

TEST_F( FileTests, Close ){
    io::File file( loop );

    file.open( file_name ).then([&](){
        return file.close();
    });

    loop.run();
}

// -------------------------------------------------------------------------- //

TEST_F( FileTests, Write ){
    io::File file( loop );

    file.open( file_name )
        .then([&](){ return file.write( contents ); })
        .then([&](){ return file.close();           })
    ;

    loop.run();

    std::ifstream test_stream( file_name );
    std::string test_string;
    std::getline( test_stream, test_string );

    EXPECT_EQ(
        memory::Buffer( test_string.begin(), test_string.end() ),
        contents
    );
}

// -------------------------------------------------------------------------- //

TEST_F( FileTests, Read ){
    io::File write_file( loop );
    io::File read_file( loop );
    bool made_it_to_the_end = false;
    write_file
        .open( file_name )
        .then([&](){ return write_file.write( contents );       })
        .then([&](){ return write_file.close();                 })
        .then([&](){ return read_file.open( file_name );        })
        .then([&](){ return read_file.read( contents.size() );  })
        .then([&]( memory::Buffer&& data ){
            EXPECT_EQ( contents, data );
            made_it_to_the_end = true;
        });
    ;

    loop.run();

    EXPECT_TRUE( made_it_to_the_end );
}

}
}
