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

// ---------------------------------------------------------------------------------------------- //

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

template<typename Func>
struct is_tuple_callable;

/// @brief Determines if the given functional type is compatible with the arguments from in a tuple.
///
/// @tparam Func A type to test for compatibility with the tuple's contents.
/// @tparam Args A tuple to check the contents of.
template<typename Func, typename... Args>
struct is_tuple_callable<Func(std::tuple<Args...>)> : public is_callable<Func(Args...)> {};

// ---------------------------------------------------------------------------------------------- //

namespace _details {
    /// @internal
    /// @brief Performs recursive expansion of a tuple as the arguments to a function.
    ///
    /// @tparam N The number of remaining parameters to expand.
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

    /// @internal
    /// @brief Sentinel implementation, performs actual function call with expanded arguments.
    template<>
    struct tuple_call<0> {
        template<typename Func, typename... TupleArgs, typename... Args>
        static auto apply(Func&& func, const std::tuple<TupleArgs...>&, Args&&... args){
            return func(std::forward<Args>(args)...);
        }
    };
}

// ---------------------------------------------------------------------------------------------- //

/// @brief Calls the given functor using the contents of the tuple.
///
/// @par Example
/// @code{.cpp}
///     auto f = [](int a, int b, float c){ return a * b; };
///     auto t = std::make_tuple(2, 42, 3.14);
///     int x = lw::trait::apply(t, f);
///     // x == 84
/// @endcode
///
/// @tparam Func The functor type which will be called.
/// @tparam Args The types of the contents of the tuple.
///
/// @param args The tuple containing the function arguments to use.
/// @param func The functor to call with the contents of the provided tuple.
///
/// @return The value returned from executing `func` with the contents of `args`.
template<typename Func, typename... Args>
auto apply(const std::tuple<Args...>& args, Func&& func){
    return _details::tuple_call<sizeof...(Args)>::apply(std::forward<Func>(func), args);
}

}
}
