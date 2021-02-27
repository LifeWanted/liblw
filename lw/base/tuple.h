#pragma once

#include <cstdint>
#include <tuple>
#include <type_traits>

namespace lw {
namespace internal {

template <std::int64_t idx, typename T, typename Tuple>
struct TupleMemberIndex;

template <std::int64_t idx, typename T>
struct TupleMemberIndex<idx, T, std::tuple<>>:
  public std::integral_constant<std::int64_t, -1>
{};

template <std::int64_t idx, typename T, typename... Rest>
struct TupleMemberIndex<idx, T, std::tuple<T, Rest...>>:
  public std::integral_constant<std::int64_t, idx>
{};

template <std::int64_t idx, typename T, typename Member, typename... Rest>
struct TupleMemberIndex<idx, T, std::tuple<Member, Rest...>>:
  public std::integral_constant<
    std::int64_t,
    TupleMemberIndex<idx + 1, T, std::tuple<Rest...>>::value
  >
{};

template <typename Input, typename Result>
struct TupleFlatten;

template <typename T, typename... Rest, typename... Results>
struct TupleFlatten<std::tuple<T, Rest...>, std::tuple<Results...>>:
  public TupleFlatten<
    std::tuple<Rest...>,
    typename TupleFlatten<T, std::tuple<Results...>>::type
  >
{};

template <typename T, typename... Results>
struct TupleFlatten<T, std::tuple<Results...>> {
  typedef std::tuple<Results..., T> type;
};

template <typename... Results>
struct TupleFlatten<std::tuple<>, std::tuple<Results...>> {
  typedef std::tuple<Results...> type;
};

template <typename Input, typename Result>
struct DedupTuple;

template <typename T, typename... Rest, typename... Results>
struct DedupTuple<std::tuple<T, Rest...>, std::tuple<Results...>> {
  typedef std::conditional_t<
    TupleMemberIndex<0, T, std::tuple<Results...>>::value == -1,
    typename DedupTuple<std::tuple<Rest...>, std::tuple<Results..., T>>::type,
    typename DedupTuple<std::tuple<Rest...>, std::tuple<Results...>>::type
  > type;
};

template <typename... Results>
struct DedupTuple<std::tuple<>, std::tuple<Results...>> {
  typedef std::tuple<Results...> type;
};

}

/**
 * Determines if the type T is a member of the given std::tuple.
 */
template <typename T, typename Tuple>
struct IsTupleMember;

template <typename T, typename... Members>
struct IsTupleMember<T, std::tuple<Members...>>:
  public std::bool_constant<
    internal::TupleMemberIndex<0, T, std::tuple<Members...>>::value != -1
  >
{};

template <typename T, typename Tuple>
constexpr bool is_tuple_member_v = IsTupleMember<T, Tuple>::value;

/**
 * Removes nested tuples, promoting their members to members of the containing
 * tuple.
 *
 * For example, the tuple
 * `std::tuple<char, std::tuple<std::tuple<int, char>, float>>` would flatten to
 * `std::tuple<char, int, char, float>`. Note that elements are not
 * deduplicated. The only change is the recursive removal of decendent tuples.
 */
template <typename Tuple>
struct FlatTuple;

template <typename... Types>
struct FlatTuple<std::tuple<Types...>> {
  typedef typename internal::TupleFlatten<
    std::tuple<Types...>,
    std::tuple<>
  >::type type;
};

template <typename Tuple>
using flat_tuple_t = typename FlatTuple<Tuple>::type;

/**
 * Removes duplicated members of the given `std::tuple`.
 */
template <typename Tuple>
struct DeduplicateTuple;

template <typename... Types>
struct DeduplicateTuple<std::tuple<Types...>> {
  typedef typename internal::DedupTuple<
    std::tuple<Types...>,
    std::tuple<>
  >::type type;
};

template <typename Tuple>
using deduplicate_tuple_t = typename DeduplicateTuple<Tuple>::type;

/**
 * Applies the given type modification to every member of the tuple.
 */
template <template <typename T> typename TypeMod, typename Tuple>
struct ApplyTypeModification;

template <template <typename T> typename TypeMod, typename... Types>
struct ApplyTypeModification<TypeMod, std::tuple<Types...>> {
  typedef std::tuple<typename TypeMod<Types>::type...> type;
};

template <template <typename T> typename TypeMod, typename Tuple>
using apply_type_modification_t =
  typename ApplyTypeModification<TypeMod, Tuple>::type;

/**
 * Flattens, deduplicates, and removes type modifications for the tuple members.
 */
template <typename Tuple>
struct SanitizeTuple {
  typedef deduplicate_tuple_t<
    apply_type_modification_t<std::remove_cvref, flat_tuple_t<Tuple>>
  > type;
};

template <typename Tuple>
using sanitize_tuple_t = typename SanitizeTuple<Tuple>::type;

}
