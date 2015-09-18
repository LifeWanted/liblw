
#include <gtest/gtest.h>
#include <functional>
#include <string>
#include <tuple>

#include "lw/trait.hpp"

namespace lw {
namespace tests {

const auto& int_func = [](int){};
const auto& auto_func = [](auto){};
const auto& int_float_func = [](int, float){};

struct FunctionTests : public testing::Test {
    typedef decltype(int_func)          int_func_type;
    typedef decltype(auto_func)         auto_func_type;
    typedef decltype(int_float_func)    int_float_func_type;

    typedef std::tuple<int>                 int_tuple_type;
    typedef std::tuple<short>               short_tuple_type;
    typedef std::tuple<float>               float_tuple_type;
    typedef std::tuple<std::string>         string_tuple_type;
    typedef std::tuple<int, int>            int_int_tuple_type;
    typedef std::tuple<int, float>          int_float_tuple_type;
    typedef std::tuple<float, int>          float_int_tuple_type;
    typedef std::tuple<float, float>        float_float_tuple_type;
    typedef std::tuple<int, std::string>    int_string_tuple_type;
};

// ---------------------------------------------------------------------------------------------- //

TEST_F(FunctionTests, Apply){
    int res = trait::apply(std::make_tuple(2, 42, 3.14f), [&](int a, int b, float c){
        EXPECT_EQ(2, a);
        EXPECT_EQ(42, b);
        EXPECT_EQ(3.14f, c);
        return a * b;
    });
    EXPECT_EQ(84, res);
}

// ---------------------------------------------------------------------------------------------- //

struct IsCallableTests : public FunctionTests {};

// ---------------------------------------------------------------------------------------------- //

TEST_F(IsCallableTests, Simple){
    EXPECT_TRUE(trait::is_callable<int_func_type(int)>::value);
    EXPECT_TRUE(trait::is_callable<int_func_type(float)>::value);
    EXPECT_TRUE(trait::is_callable<int_func_type(short)>::value);

    EXPECT_FALSE(trait::is_callable<int_func_type(std::string)>::value);
    EXPECT_FALSE(trait::is_callable<int_func_type(int, std::string)>::value);
    EXPECT_FALSE(trait::is_callable<int_func_type(int, int)>::value);
}

// ---------------------------------------------------------------------------------------------- //

TEST_F(IsCallableTests, Templated){
    EXPECT_TRUE(trait::is_callable<auto_func_type(int)>::value);
    EXPECT_TRUE(trait::is_callable<auto_func_type(float)>::value);
    EXPECT_TRUE(trait::is_callable<auto_func_type(short)>::value);
    EXPECT_TRUE(trait::is_callable<auto_func_type(std::string)>::value);

    EXPECT_FALSE(trait::is_callable<auto_func_type(int, std::string)>::value);
    EXPECT_FALSE(trait::is_callable<auto_func_type(int, int)>::value);
}

// ---------------------------------------------------------------------------------------------- //

TEST_F(IsCallableTests, MultipleParameters){
    EXPECT_TRUE(trait::is_callable<int_float_func_type(int, int)>::value);
    EXPECT_TRUE(trait::is_callable<int_float_func_type(int, float)>::value);
    EXPECT_TRUE(trait::is_callable<int_float_func_type(float, int)>::value);
    EXPECT_TRUE(trait::is_callable<int_float_func_type(float, float)>::value);

    EXPECT_FALSE(trait::is_callable<int_float_func_type(int)>::value);
    EXPECT_FALSE(trait::is_callable<int_float_func_type(float)>::value);
    EXPECT_FALSE(trait::is_callable<int_float_func_type(short)>::value);
    EXPECT_FALSE(trait::is_callable<int_float_func_type(std::string)>::value);
    EXPECT_FALSE(trait::is_callable<int_float_func_type(int, std::string)>::value);
}

// ---------------------------------------------------------------------------------------------- //

struct IsTupleCallableTests : public FunctionTests {};

// ---------------------------------------------------------------------------------------------- //

TEST_F(IsTupleCallableTests, Simple){
    auto intFunc = [](int i){};
    EXPECT_TRUE(trait::is_tuple_callable<int_func_type(int_tuple_type)>::value);
    EXPECT_TRUE(trait::is_tuple_callable<int_func_type(float_tuple_type)>::value);
    EXPECT_TRUE(trait::is_tuple_callable<int_func_type(short_tuple_type)>::value);

    EXPECT_FALSE(trait::is_tuple_callable<int_func_type(string_tuple_type)>::value);
    EXPECT_FALSE(trait::is_tuple_callable<int_func_type(int_string_tuple_type)>::value);
    EXPECT_FALSE(trait::is_tuple_callable<int_func_type(int_float_tuple_type)>::value);
}

// ---------------------------------------------------------------------------------------------- //

TEST_F(IsTupleCallableTests, Templated){
    EXPECT_TRUE(trait::is_tuple_callable<auto_func_type(int_tuple_type)>::value);
    EXPECT_TRUE(trait::is_tuple_callable<auto_func_type(float_tuple_type)>::value);
    EXPECT_TRUE(trait::is_tuple_callable<auto_func_type(short_tuple_type)>::value);
    EXPECT_TRUE(trait::is_tuple_callable<auto_func_type(string_tuple_type)>::value);

    EXPECT_FALSE(trait::is_tuple_callable<auto_func_type(int_string_tuple_type)>::value);
    EXPECT_FALSE(trait::is_tuple_callable<auto_func_type(int_float_tuple_type)>::value);
}

// ---------------------------------------------------------------------------------------------- //

TEST_F(IsTupleCallableTests, MultipleParameters){
    EXPECT_TRUE(trait::is_tuple_callable<int_float_func_type(int_int_tuple_type)>::value);
    EXPECT_TRUE(trait::is_tuple_callable<int_float_func_type(int_float_tuple_type)>::value);
    EXPECT_TRUE(trait::is_tuple_callable<int_float_func_type(float_int_tuple_type)>::value);
    EXPECT_TRUE(trait::is_tuple_callable<int_float_func_type(float_float_tuple_type)>::value);

    EXPECT_FALSE(trait::is_tuple_callable<int_float_func_type(int_tuple_type)>::value);
    EXPECT_FALSE(trait::is_tuple_callable<int_float_func_type(float_tuple_type)>::value);
    EXPECT_FALSE(trait::is_tuple_callable<int_float_func_type(short_tuple_type)>::value);
    EXPECT_FALSE(trait::is_tuple_callable<int_float_func_type(string_tuple_type)>::value);
    EXPECT_FALSE(trait::is_tuple_callable<int_float_func_type(int_string_tuple_type)>::value);
}

}
}
