#pragma once

#include <type_traits>
#include <tuple>

namespace lw {
namespace trait {

/// @brief A type which is always void.
///
/// Useful for wrapping up SFINAE template overloads.
///
/// @tparam T Any type, any type at all.
template<typename T>
using void_type = void;

/// @brief Determines if the functional type is callable with the given arguments.
///
/// @tparam Func A type to test for `operator()(Args...)` support.
/// @tparam Args The arguments to test the function against.
template<typename Func, typename Args = void>
struct is_callable : public std::false_type {};

template<typename Func, typename... Args>
struct is_callable<Func(Args...), void_type<typename std::result_of<Func(Args...)>::type>> :
    public std::true_type
{};

/// @brief Determines if the given functional type is compatible with the arguments from in a tuple.
///
/// @tparam Func A type to test for compatibility with the tuple's contents.
/// @tparam Args A tuple to check the contents of.
template<typename Func, typename Args>
struct is_tuple_callable;

template<typename Func, typename... Args>
struct is_tuple_callable<Func, std::tuple<Args...>> : public is_callable<Func(Args...)> {};

namespace _details {
    template<std::size_t N>
    struct tuple_call {
        template<typename Func, typename... TupleArgs, typename... Args>
        static auto apply(Func&& func, const std::tuple<TupleArgs...>& tuple, Args&&... args){
            return tuple_call<N - 1>::apply(
                std::forward<Func>(func),
                tuple,
                std::get<N - 1>(tuple),
                std::forward<Args>(args)...
            );
        }
    };

    template<>
    struct tuple_call<0> {
        template<typename Func, typename... TupleArgs, typename... Args>
        static auto apply(Func&& func, const std::tuple<TupleArgs...>&, Args&&... args){
            return func(std::forward<Args>(args)...);
        }
    };
}

/// @brief Calls the given functor using the contents of the tuple.
///
/// @par Example
/// @code{.cpp}
///     auto f = [](int a, int b, float c){ return a * b; };
///     auto t = std::make_tuple(2, 42, 3.14);
///     int x = lw::traits::apply(f, t);
///     // x == 84
/// @endcode
///
/// @tparam Func The functor type which will be called.
/// @tparam Args The types of the contents of the tuple.
///
/// @param func The functor to call with the contents of the provided tuple.
/// @param args The tuple containing the function arguments to use.
///
/// @return The value returned from executing `func` with the contents of `args`.
template<typename Func, typename... Args>
auto apply(Func&& func, const std::tuple<Args...>& args){
    return _details::tuple_call<sizeof...(Args)>::apply(std::forward<Func>(func), args);
}

}
}
