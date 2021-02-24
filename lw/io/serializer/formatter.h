#pragma once

#include <cstdint>
#include <string_view>

namespace lw::io {

/**
 * Interface for serialization formats.
 *
 * New formats to be used with `lw::io::Serializer` must implement this
 * interface.
 *
 * @see lw::mime::JSONSerializationFormatter
 */
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

}
