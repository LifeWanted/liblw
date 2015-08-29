
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
    emitter.on(emitter.test_event, [&](){ emitterCalled = true; });
    EXPECT_FALSE(emitterCalled);

    emitter.emit(emitter.test_event);
    EXPECT_TRUE(emitterCalled);
}

// ---------------------------------------------------------------------------------------------- //

TEST_F(EmitterTests, EmitValueEvent){
    std::string value = "initial value";
    emitter.on(emitter.foobar_event, [&](const std::string& val){ value = val; });
    EXPECT_EQ("initial value", value);

    emitter.emit(emitter.foobar_event, "fizz bang");
    EXPECT_EQ("fizz bang", value);
}

}
}
