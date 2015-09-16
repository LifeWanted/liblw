#pragma once

#include <tuple>

namespace lw {
namespace trait {

namespace _details {
    /// @internal
    /// @brief Recursively expands a tuple with a function call for each value.
    ///
    /// @tparam I       The current position in the iteration.
    /// @tparam Total   The total number of elements contained in the tuple.
    /// @tparam Types   The types contained in the tuple.
    template<std::size_t I, std::size_t Total, typename... Types>
    struct for_each_impl {
        template<typename F>
        inline static void exec(std::tuple<Types...>& tup, F&& func){
            func(std::get<I>(tup));
            for_each_impl<I + 1, Total, Types...>::exec(tup, func);
        }

        template<typename F>
        inline static void exec(const std::tuple<Types...>& tup, F&& func){
            func(std::get<I>(tup));
            for_each_impl<I + 1, Total, Types...>::exec(tup, func);
        }
    };

    template<std::size_t Total, typename... Types>
    struct for_each_impl<Total, Total, Types...> {
        template<typename F>
        inline static void exec(std::tuple<Types...>& tup, F&& func){}

        template<typename F>
        inline static void exec(const std::tuple<Types...>& tup, F&& func){}
    };
}

// ---------------------------------------------------------------------------------------------- //

/// @brief Executes the given function with every value contained in the tuple.
///
/// @par Example
/// @code{.cpp}
///     std::tuple<float, int, std::string> tup = std::make_tuple(3.14, 42, "Hello, World!");
///     for_each(tup, [](const auto& val){ std::cout << val << std::endl; });
/// @endcode
///
/// @tparam F       The functor type to call with each value.
/// @tparam Types   The types contained in the tuple.
///
/// @param tup  The tuple to iterate over.
/// @param func The function to call with each value.
template<typename F, typename... Types>
void for_each(std::tuple<Types...>& tup, F&& func){
    _details::for_each_impl<0, sizeof...(Types), Types...>::exec(tup, std::forward<F>(func));
}

/// @copydoc lw::trait::for_each
template<typename F, typename... Types>
void for_each(const std::tuple<Types...>& tup, F&& func){
    _details::for_each_impl<0, sizeof...(Types), Types...>::exec(tup, std::forward<F>(func));
}

}
}
