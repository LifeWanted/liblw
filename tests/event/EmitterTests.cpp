
#include <gtest/gtest.h>

#include "lw/error.hpp"
#include "lw/event.hpp"

namespace lw {
namespace tests {

namespace _details {
    LW_DECLARE_EVENTS(test, error, foobar)
    LW_DEFINE_EMITTER(
        TestEmitter,
        (test),
        (error, const error::Exception&),
        (foobar, const std::string&)
    );
}

// ---------------------------------------------------------------------------------------------- //

struct EmitterTests : public testing::Test {
    class MyEmitter : public _details::TestEmitter {};

    MyEmitter emitter;
};

// ---------------------------------------------------------------------------------------------- //

TEST_F(EmitterTests, EmitVoidEvent){
    bool emitterCalled = false;
    emitter.on(MyEmitter::event::test, [&](){ emitterCalled = true; });
    EXPECT_FALSE(emitterCalled);

    emitter.emit(MyEmitter::event::test);
    EXPECT_TRUE(emitterCalled);
}

// ---------------------------------------------------------------------------------------------- //

TEST_F(EmitterTests, EmitValueEvent){
    std::string value = "initial value";
    emitter.on(MyEmitter::event::foobar, [&](const std::string& val){ value = val; });

    emitter.emit(MyEmitter::event::foobar, "fizz bang");
    EXPECT_EQ("fizz bang", value);
}

}
}
