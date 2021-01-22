#pragma once

#include <concepts>
#include <type_traits>
#include <utility>

#include "lw/base/concepts.h"
#include "lw/co/future.h"

namespace lw::io {

template <typename T>
struct Serialize;

class Serializer;
class SerializedValue;

struct ObjectTag;
struct ListTag;

namespace internal {

template <typename T>
struct PairTypes;

template <typename Key, typename Value>
struct PairTypes<std::pair<Key, Value>> {
  typedef Key first_type;
  typedef Value second_type;
};

template <typename T, typename SerializationResult>
concept BasicSerializable = requires(Serializer& s, const T& a) {
  typename Serialize<T>;
  { Serialize<T>{}.serialize(s, a) } -> std::same_as<SerializationResult>;
};

template <typename T, typename SerializationResult>
concept ListSerializationTagged =
  requires() {
    typename Serialize<T>;
    typename Serialize<T>::serialization_category;
    { typename Serialize<T>::serialization_category{} } -> std::same_as<ListTag>;
  } &&
  BasicSerializable<T, SerializationResult>;

template <typename T, typename SerializationResult>
concept ListSerializationCapable =
  ForwardIterable<T> &&
  requires(const T& a) {
    { *a.begin() } -> BasicSerializable<SerializationResult>;
  };

template <typename T, typename SerializationResult>
concept ObjectSerializationTagged =
  requires() {
    typename Serialize<T>;
    typename Serialize<T>::serialization_category;
    {
      typename Serialize<T>::serialization_category{}
    } -> std::same_as<ObjectTag>;
  } &&
  BasicSerializable<T, SerializationResult>;

template <typename T, typename SerializationResult>
concept ObjectSerializationCapable =
  ForwardIterable<T> &&
  requires(const T& a) {
    typename PairTypes<decltype(*a.begin())>;
    { a.begin()->first } -> BasicSerializable<SerializationResult>;
    { a.begin()->second } -> BasicSerializable<SerializationResult>;
  };

}

template <typename T>
concept NumericSerializable = std::integral<T> || std::floating_point<T>;

template <typename T>
concept StringSerializable = ForwardIterableAs<T, char>;

template <typename T>
concept DirectlySerializable =
  NumericSerializable<T> ||
  StringSerializable<T> ||
  std::same_as<T, bool> ||
  std::same_as<T, std::nullptr_t>;

template <typename T>
concept ListSerializable =
  internal::ListSerializationTagged<T, void> ||
  internal::ListSerializationCapable<T, void>;

template <typename T>
concept AsyncListSerializable =
  internal::ListSerializationTagged<T, co::Future<void>> ||
  internal::ListSerializationCapable<T, co::Future<void>>;

template <typename T>
concept ObjectSerializable =
  internal::ObjectSerializationTagged<T, void> ||
  internal::ObjectSerializationCapable<T, void>;

template <typename T>
concept AsyncObjectSerializable =
  internal::ObjectSerializationTagged<T, co::Future<void>>; // Must be tagged.

template <typename T>
concept Serializeable = internal::BasicSerializable<T, void>;

template <typename T>
concept AsyncSerializeable = internal::BasicSerializable<T, co::Future<void>>;

template <typename T>
concept Deserializeable = requires() {
  typename Serialize<T>;
  {
    Serialize<T>{}.deserialize(std::declval<SerializedValue>())
  } -> std::same_as<T>;
};

}
