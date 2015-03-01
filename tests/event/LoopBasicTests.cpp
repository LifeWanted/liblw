
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

TEST_F( LoopBasicTests)

}
}
