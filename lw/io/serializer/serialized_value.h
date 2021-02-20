#pragma once

#include <string_view>

namespace lw::io {

class DeserializationParser;

class SerializedValue {
public:
  SerializedValue(DeserializationParser& parser, std::string_view value);

  bool has(std::string_view key);

  template <typename T>
  T get(std::string_view key);

  template <typename T>
  T get(std::string_view key, T default_value);

  template <typename T>
  T as();
};

}
