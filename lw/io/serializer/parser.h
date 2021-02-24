#pragma once

#include <cstdint>
#include <memory>
#include <string_view>

namespace lw::io {

class DeserializationToken {
public:
  DeserializationToken() = default;
  virtual ~DeserializationToken() = default;

  virtual std::size_t size() const = 0;

  virtual bool is_null() const = 0;
  virtual bool is_boolean() const = 0;
  virtual bool is_char() const = 0;
  virtual bool is_signed_integer() const = 0;
  virtual bool is_unsigned_integer() const = 0;
  virtual bool is_floating_point() const = 0;
  virtual bool is_string() const = 0;
  virtual bool is_list() const = 0;
  virtual bool is_object() const = 0;

  virtual bool get_boolean() const = 0;
  virtual char get_char() const = 0;
  virtual std::int64_t get_signed_integer() const = 0;
  virtual std::uint64_t get_unsigned_integer() const = 0;
  virtual double get_floating_point() const = 0;
  virtual std::string_view get_string() const = 0;

  virtual bool has_index(std::size_t idx) const = 0;
  virtual const DeserializationToken& get_index(std::size_t idx) const = 0;

  virtual bool has_key(std::string_view key) const = 0;
  virtual const DeserializationToken& get_key(std::string_view key) const = 0;
};

class DeserializationParser {
public:
  DeserializationParser() = default;
  virtual ~DeserializationParser() = default;

  virtual std::unique_ptr<DeserializationToken> parse(
    std::string_view str
  ) const = 0;
};

}
