#pragma once

#include <concepts>
#include <memory>
#include <string_view>

#include "lw/co/future.h"
#include "lw/io/serializer/concepts.h"
#include "lw/io/serializer/formatter.h"

namespace lw::io {

/**
 * Tag to be used by specializations of `::lw::io::Serialize` to trigger object
 * formatting in the serializer.
 *
 * For example, in JSON formatting, this tag results in `{` being inserted
 * before calling the `Serialize::serialize` method, and `}` being inserted
 * afterwards. When in object-formatting, the JSON formatter will also know to
 * expect 2 values and insert a `:` between them along with a `,` before the 2nd
 * through nth pairs.
 *
 * `Serialize` specializations with `typedef ObjectTag serialization_category`
 * match the `ObjectSerializable` concept.
 *
 * @example{.cpp}
 *  template<>
 *  struct Serialize<MyObjectClass> {
 *    typedef ObjectTag serialization_category; // <-- Object Tagged
 *
 *    void serialize(Serializer& serializer, const MyObjectClass&);
 *    MyObjectClass deserialize(SerializedValue&);
 *  };
 */
struct ObjectTag {};

/**
 * Tag to be used by specializations of `::lw::io::Serialize` to trigger list
 * formatting in the serializer.
 *
 * For example, in JSON formatting, this tag results in `[` being inserted
 * before calling the `Serialize::serialize` method, and `]` being inserted
 * afterwards. When in list-formatting, the JSON formatter will also know to
 * put a `,` before the 2nd through nth values.
 *
 * `Serialize` specializations with `typedef ListTag serialization_category`
 * match the `ListSerializable` concept.
 *
 * @example{.cpp}
 *  template<>
 *  struct Serialize<MyListClass> {
 *    typedef ListTag serialization_category; // <-- List Tagged
 *
 *    void serialize(Serializer& serializer, const MyListClass&);
 *    MyListClass deserialize(SerializedValue&);
 *  };
 */
struct ListTag {};

/**
 * Specialize this type for any data types you wish to serialize. The
 * specialization may have one or both of a `serialize` and `deserialize` method
 * defined.
 *
 * @example{.cpp}
 *  struct Foo {
 *    int bar;
 *    float baz;
 *  };
 *
 *  namespace lw::io {
 *  template <>
 *  struct Serialize<Foo> {
 *    typedef ObjectTag serialization_category;
 *
 *    void serialize(Serializer& serializer, const Foo& foo) {
 *      serializer.write("bar", foo.bar);
 *      serializer.write("baz", foo.baz);
 *    }
 *
 *    Foo deserialize(SerializedValue& val) {
 *      return {
 *        .bar = val.get<int>("bar"),
 *        .baz = val.get<float>("baz")
 *      };
 *    }
 *  };
 *  }
 */
template <typename T>
struct Serialize;

// -------------------------------------------------------------------------- //

/**
 * Type-aware serialization interface that bridges the gap between serializable
 * types and formatters.
 *
 * Serializer::write comes in 4 forms:
 *  - `void write(Serializable value)`
 *  - `void write(Serializable key, Serializable value)`
 *  - `co::Future<void> write(AsyncSerializable value)`
 *  - `co::Future<void> write(Serializable key, AsyncSerializable value)`
 *
 * Where `Serializable` and `AsyncSerializable` match their respective concepts.
 * There are also key-value versions that take `std::pair` to make working with
 * STL maps easier.
 *
 * The key-value versions should only be used by `Serialize` specializations
 * that specify `typedef ObjectTag serialization_category`.
 */
class Serializer {
public:
  Serializer(std::unique_ptr<SerializationFormatter> formatter):
    _formatter{std::move(formatter)}
  {}

  void write(std::nullptr_t) {
    _formatter->put_null();
  }

  void write(bool b) {
    _formatter->put_boolean(b);
  }

  void write(char c) {
    _formatter->put_char(c);
  }

  template <std::signed_integral Number>
  void write(Number n) {
    _formatter->put_signed_integer(n);
  }

  template <std::unsigned_integral Number>
  void write(Number n) {
    _formatter->put_unsigned_integer(n);
  }

  template <std::floating_point Number>
  void write(Number n) {
    _formatter->put_floating_point(n);
  }

  void write(const char* s) {
    _formatter->put_string(s);
  }

  template <StringSerializable String>
  void write(const String& s) {
    _formatter->put_string(s);
  }

  template <ListSerializable List>
  void write(const List& l) {
    _formatter->start_list();
    _serialize_list(l);
    _formatter->end_list();
  }

  template <AsyncListSerializable List>
  co::Future<void> write(const List& l) {
    _formatter->start_list();
    co_await _serialize_list(l);
    _formatter->end_list();
  }

  template <ObjectSerializable Object>
  void write(const Object& o) {
    _formatter->start_object();
    _serialize_object(o);
    _formatter->end_object();
  }

  template <AsyncObjectSerializable Object>
  co::Future<void> write(const Object& o) {
    _formatter->start_object();
    co_await _serialize_object(o);
    _formatter->end_object();
  }

  template <AsyncSerializeable Value>
  co::Future<void> write(const Value& value) {
    Serialize<Value> s;
    co_await s.serialize(*this, value);
  }

  /**
   * Specialization for key-value pairs using `std::pair`.
   *
   * @see ::lw::io::ObjectTag
   */
  template <Serializeable Key, Serializeable Value>
  void write(const std::pair<Key, Value>& pair) {
    _formatter->start_pair_key();
    write(pair.first);
    _formatter->end_pair_key();
    write(pair.second);
    _formatter->end_pair();
  }

  /**
   * Specialization for key-value pairs using `std::pair`.
   *
   * @see ::lw::io::ObjectTag
   */
  template <Serializeable Key, AsyncSerializeable Value>
  co::Future<void> write(const std::pair<Key, Value>& pair) {
    _formatter->start_pair_key();
    write(pair.first);
    _formatter->end_pair_key();
    co_await write(pair.second);
    _formatter->end_pair();
  }

  /**
   * Key-value pair writing for custom object serialization.
   *
   * @see ::lw::io::ObjectTag
   */
  template <Serializeable Key, Serializeable Value>
  void write(const Key& key, const Value& value) {
    _formatter->start_pair_key();
    write(key);
    _formatter->end_pair_key();
    write(value);
    _formatter->end_pair();
  }

  /**
   * Key-value pair writing for custom object serialization.
   *
   * @see ::lw::io::ObjectTag
   */
  template <Serializeable Key, AsyncSerializeable Value>
  co::Future<void> write(const Key& key, const Value& value) {
    _formatter->start_pair_key();
    write(key);
    _formatter->end_pair_key();
    co_await write(value);
    _formatter->end_pair();
  }

private:
  template <internal::ListSerializationTagged<void> List>
  void _serialize_list(const List& l) {
    Serialize<List> s;
    s.serialize(*this, l);
  }

  template <internal::ListSerializationCapable<void> List>
  void _serialize_list(const List& l) {
    for (const auto& v : l) write(v);
  }

  template <internal::ListSerializationTagged<co::Future<void>> List>
  co::Future<void> _serialize_list(const List& l) {
    Serialize<List> s;
    co_await s.serialize(*this, l);
  }

  template <internal::ListSerializationCapable<co::Future<void>> List>
  co::Future<void> _serialize_list(const List& l) {
    for (const auto& v : l) co_await write(v);
  }

  template <internal::ObjectSerializationTagged<void> Object>
  void _serialize_object(const Object& o) {
    Serialize<Object> s;
    s.serialize(*this, o);
  }

  template <internal::ObjectSerializationCapable<void> Object>
  void _serialize_object(const Object& l) {
    for (const auto& v : l) write(v);
  }

  template <internal::ObjectSerializationTagged<co::Future<void>> Object>
  co::Future<void> _serialize_object(const Object& o) {
    Serialize<Object> s;
    co_await s.serialize(*this, o);
  }

  std::unique_ptr<SerializationFormatter> _formatter;
};

template <DirectlySerializable T>
struct Serialize<T> {
  void serialize(Serializer& serializer, const T& value) {
    serializer.write(value);
  }
};

template <ListSerializable T>
struct Serialize<T> {
  void serialize(Serializer& serializer, const T& value) {
    serializer.write(value);
  }
};

template <AsyncListSerializable T>
struct Serialize<T> {
  co::Future<void> serialize(Serializer& serializer, const T& value) {
    return serializer.write(value);
  }
};

template <ObjectSerializable T>
struct Serialize<T> {
  void serialize(Serializer& serializer, const T& value) {
    serializer.write(value);
  }
};

// -------------------------------------------------------------------------- //

class Deserializer {
public:
};

}
