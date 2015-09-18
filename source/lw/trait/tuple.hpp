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

// ---------------------------------------------------------------------------------------------- //

namespace _details {
    /// @internal
    /// @brief Iterates over the `DirtyTuple`, copying types over to the `CleanTuple`.
    ///
    /// @tparam CleanTuple  The tuple to fill with the clean types.
    /// @tparam Remove      The type to remove from `DirtyTuple`.
    /// @tparam DirtyTuple  The tuple to remove the type from.
    template<typename CleanTuple, typename Remove, typename DirtyTuple>
    struct remove_type_impl;

    template<typename... CleanTypes, typename Remove, typename T, typename... Types>
    struct remove_type_impl<std::tuple<CleanTypes...>, Remove, std::tuple<T, Types...>> :
        public remove_type_impl<std::tuple<CleanTypes..., T>, Remove, std::tuple<Types...>>
    {};

    template<typename... CleanTypes, typename Remove, typename... Types>
    struct remove_type_impl<std::tuple<CleanTypes...>, Remove, std::tuple<Remove, Types...>> :
        public remove_type_impl<std::tuple<CleanTypes...>, Remove, std::tuple<Types...>>
    {};

    template<typename... CleanTypes, typename Remove>
    struct remove_type_impl<std::tuple<CleanTypes...>, Remove, std::tuple<>> {
        typedef std::tuple<CleanTypes...> type;
    };
}

// ---------------------------------------------------------------------------------------------- //

/// @brief Removes all instances of a given type from a tuple.
///
/// @par Example
/// @code{.cpp}
///     typedef std::tuple<float, int, float, std::string, float> my_tuple_type;
///     typedef typename lw::trait::remove_type<float, my_tuple_type>::type cleaned_tuple_type;
///     std::is_same<std::tuple<int, std::string>, cleaned_tuple_type>::value == true;
/// @endcode
///
/// @tparam Remove  The type to remove from the tuple.
/// @tparam Tuple   The tuple type to clean.
template<typename Remove, typename Tuple>
struct remove_type : public _details::remove_type_impl<std::tuple<>, Remove, Tuple> {};

}
}
