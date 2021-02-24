#pragma once

#include <concepts>
#include <string_view>

#include "lw/io/serializer/concepts.h"
#include "lw/io/serializer/parser.h"

namespace lw::io {

class SerializedValue {
public:
  SerializedValue() = delete;
  SerializedValue(const SerializedValue&) = delete;
  SerializedValue(SerializedValue&&) = delete;
  SerializedValue& operator=(const SerializedValue&) = delete;
  SerializedValue& operator=(SerializedValue&&) = delete;

  explicit SerializedValue(const DeserializationToken& token): _token{token} {}

  bool has(std::string_view key) const { return _token.has_key(key); }
  bool has(std::size_t index) const { return _token.has_index(index); }

  bool is_null() const { return _token.is_null(); }
  bool is_boolean() const { return _token.is_boolean(); }
  bool is_char() const { return _token.is_char(); }
  bool is_signed_integer() const { return _token.is_signed_integer(); }
  bool is_unsigned_integer() const { return _token.is_unsigned_integer(); }
  bool is_floating_point() const { return _token.is_floating_point(); }
  bool is_string() const { return _token.is_string(); }
  bool is_list() const { return _token.is_list(); }
  bool is_object() const { return _token.is_object(); }

  template <typename T>
  T get(std::string_view key) const {
    return SerializedValue{_token.get_key(key)}.as<T>();
  }

  template <typename T>
  T get(std::string_view key, T default_value) const {
    if (_token.has_key(key)) {
      return SerializedValue{_token.get_key(key)}.as<T>();
    }
    return std::move(default_value);
  }

  template <typename T>
  T get(std::size_t index) const {
    return SerializedValue{_token.get_index(index)}.as<T>();
  }

  template <typename T>
  T get(std::size_t index, T default_value) const {
    if (_token.has_index(index)) {
      return SerializedValue{_token.get_index(index)}.as<T>();
    }
    return std::move(default_value);
  }

  template <std::same_as<std::nullptr_t> T>
  T as() const { return nullptr; }

  template <std::same_as<bool> T>
  T as() const { return _token.get_boolean(); }

  template <std::same_as<char> T>
  T as() const { return _token.get_char(); }

  template <SignedIntegerSerializable T>
  T as() const { return _token.get_signed_integer(); }

  template <UnsignedIntegerSerializable T>
  T as() const { return _token.get_unsigned_integer(); }

  template <std::floating_point T>
  T as() const { return _token.get_floating_point(); }

  template <std::same_as<std::string_view> T>
  T as() const { return _token.get_string(); }

  template <Deserializeable T>
  T as() const { return Serialize<T>{}.deserialize(*this); }

private:
  const DeserializationToken& _token;
};

}
