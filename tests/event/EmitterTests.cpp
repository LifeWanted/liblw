
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
    bool emitter_called = false;
    emitter.on(emitter.test_event, [&](){ emitter_called = true; });
    EXPECT_FALSE(emitter_called);

    emitter.emit(emitter.test_event);
    EXPECT_TRUE(emitter_called);
}

// ---------------------------------------------------------------------------------------------- //

TEST_F(EmitterTests, EmitValueEvent){
    std::string value = "initial value";
    emitter.on(emitter.foobar_event, [&](const std::string& val){ value = val; });
    EXPECT_EQ("initial value", value);

    emitter.emit(emitter.foobar_event, "fizz bang");
    EXPECT_EQ("fizz bang", value);
}

// ---------------------------------------------------------------------------------------------- //

TEST_F(EmitterTests, ClearEvent){
    int test_call_count = 0;
    int foobar_call_count = 0;
    emitter.on(emitter.test_event, [&](){ ++test_call_count; });
    emitter.on(emitter.foobar_event, [&](const std::string&){ ++foobar_call_count; });

    EXPECT_EQ(0, test_call_count);
    EXPECT_EQ(0, foobar_call_count);
    emitter.emit(emitter.test_event);
    emitter.emit(emitter.foobar_event, "");
    EXPECT_EQ(1, test_call_count);
    EXPECT_EQ(1, foobar_call_count);

    emitter.clear(emitter.test_event);
    emitter.emit(emitter.test_event);
    emitter.emit(emitter.foobar_event, "");
    EXPECT_EQ(1, test_call_count);
    EXPECT_EQ(2, foobar_call_count);
}

// ---------------------------------------------------------------------------------------------- //

TEST_F(EmitterTests, ClearAllEvents){
    int test_call_count = 0;
    int foobar_call_count = 0;
    emitter.on(emitter.test_event, [&](){ ++test_call_count; });
    emitter.on(emitter.foobar_event, [&](const std::string&){ ++foobar_call_count; });

    EXPECT_EQ(0, test_call_count);
    EXPECT_EQ(0, foobar_call_count);
    emitter.emit(emitter.test_event);
    emitter.emit(emitter.foobar_event, "");
    EXPECT_EQ(1, test_call_count);
    EXPECT_EQ(1, foobar_call_count);

    emitter.clear();
    emitter.emit(emitter.test_event);
    emitter.emit(emitter.foobar_event, "");
    EXPECT_EQ(1, test_call_count);
    EXPECT_EQ(1, foobar_call_count);
}

// ---------------------------------------------------------------------------------------------- //

TEST_F(EmitterTests, Empty){
    EXPECT_TRUE(emitter.empty(emitter.test_event));
    EXPECT_TRUE(emitter.empty(emitter.foobar_event));

    emitter.on(emitter.test_event, [&](){});
    EXPECT_FALSE(emitter.empty(emitter.test_event));
    EXPECT_TRUE(emitter.empty(emitter.foobar_event));

    emitter.clear(emitter.test_event);
    EXPECT_TRUE(emitter.empty(emitter.test_event));
    EXPECT_TRUE(emitter.empty(emitter.foobar_event));
}

// ---------------------------------------------------------------------------------------------- //

TEST_F(EmitterTests, AllEmpty){
    EXPECT_TRUE(emitter.empty());

    emitter.on(emitter.test_event, [&](){});
    EXPECT_FALSE(emitter.empty());

    emitter.clear();
    EXPECT_TRUE(emitter.empty());
}

// ---------------------------------------------------------------------------------------------- //

TEST_F(EmitterTests, Size){
    EXPECT_EQ(0, emitter.size(emitter.test_event));
    EXPECT_EQ(0, emitter.size(emitter.foobar_event));

    emitter.on(emitter.test_event, [&](){});
    EXPECT_EQ(1, emitter.size(emitter.test_event));
    EXPECT_EQ(0, emitter.size(emitter.foobar_event));

    emitter.clear(emitter.test_event);
    EXPECT_EQ(0, emitter.size(emitter.test_event));
    EXPECT_EQ(0, emitter.size(emitter.foobar_event));
}

// ---------------------------------------------------------------------------------------------- //

TEST_F(EmitterTests, FullSize){
    EXPECT_EQ(0, emitter.size());

    emitter.on(emitter.test_event, [&](){});
    EXPECT_EQ(1, emitter.size());

    emitter.on(emitter.foobar_event, [&](const std::string&){});
    EXPECT_EQ(2, emitter.size());

    emitter.clear();
    EXPECT_EQ(0, emitter.size());
}

}
}
