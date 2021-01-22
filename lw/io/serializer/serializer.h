#pragma once

#include <concepts>
#include <cstdint>
#include <memory>
#include <string_view>

#include "lw/co/future.h"
#include "lw/io/serializer/concepts.h"

namespace lw::io {

struct ObjectTag {};
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
 *  template <>
 *  struct Serialize<Foo> {
 *    typedef ::lw::io::ObjectTag serialization_category;
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
 */
template <typename T>
struct Serialize;

class SerializedValue {
public:
  bool has(std::string_view key);

  template <typename T>
  T get(std::string_view key);

  template <typename T>
  T get(std::string_view key, T default_value);

  template <typename T>
  T as();
};

class SerializationFormatter {
public:
  SerializationFormatter() = default;
  virtual ~SerializationFormatter() = default;

  virtual void put_null() = 0;
  virtual void put_boolean(bool boolean) = 0;
  virtual void put_char(char c) = 0;
  virtual void put_signed_integer(std::int64_t number) = 0;
  virtual void put_unsigned_integer(std::uint64_t number) = 0;
  virtual void put_floating_point(double number) = 0;
  virtual void put_string(std::string_view str) = 0;

  virtual void start_list() = 0;
  virtual void end_list() = 0;
  virtual void start_object() = 0;
  virtual void end_object() = 0;
  virtual void start_pair_key() = 0;
  virtual void end_pair_key() = 0;
  virtual void end_pair() = 0;
};

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

  template <Serializeable Key, Serializeable Value>
  void write(const std::pair<Key, Value>& pair) {
    _formatter->start_pair_key();
    write(pair.first);
    _formatter->end_pair_key();
    write(pair.second);
    _formatter->end_pair();
  }

  template <Serializeable Key, AsyncSerializeable Value>
  co::Future<void> write(const std::pair<Key, Value>& pair) {
    _formatter->start_pair_key();
    write(pair.first);
    _formatter->end_pair_key();
    co_await write(pair.second);
    _formatter->end_pair();
  }

  template <Serializeable Key, Serializeable Value>
  void write(const Key& key, const Value& value) {
    _formatter->start_pair_key();
    write(key);
    _formatter->end_pair_key();
    write(value);
    _formatter->end_pair();
  }

  template <Serializeable Key, AsyncSerializeable Value>
  co::Future<void> write(const Key& key, const Value& value) {
    _formatter->start_pair_key();
    write(key);
    _formatter->end_pair_key();
    co_await write(value);
    _formatter->end_pair();
  }

  template <AsyncSerializeable Value>
  co::Future<void> write(const Value& value) {
    Serialize<Value> s;
    co_await s.serialize(*this, value);
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

}
