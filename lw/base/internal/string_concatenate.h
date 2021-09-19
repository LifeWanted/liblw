#pragma once

#include <charconv>
#include <cinttypes>
#include <concepts>
#include <cstdint>
#include <string>
#include <string_view>

namespace lw::internal {

template <std::unsigned_integral... Numbers>
constexpr std::size_t sum(Numbers&&... numbers) {
  return (numbers + ...);
}

constexpr std::size_t estimate_string_length(std::string_view str) {
  return str.size();
}

constexpr std::size_t estimate_string_length(char c) { return 1; }

constexpr std::size_t estimate_string_length(std::unsigned_integral auto num) {
  if (num < 10) return 1;
  if (num < 100) return 2;
  if (num < 1'000) return 3;
  if (num < 10'000) return 4;
  if (num < 100'000) return 5;
  if (num < 1'000'000) return 6;
  std::size_t char_count = 7;
  for (num /= 10'000'000; num > 10; num /= 10) ++char_count;
  return char_count;
}

inline void do_concatenate_one(std::string& out, std::string_view str) {
  out += str;
}

inline void do_concatenate_one(std::string& out, char c) { out += c; }

constexpr std::size_t estimate_string_length(std::signed_integral auto num) {
  std::size_t char_count = estimate_string_length(
    static_cast<std::uintmax_t>(std::abs(num))
  );
  return num < 0 ? 1 + char_count : char_count;
}

inline void do_concatenate_one(std::string& out, std::integral auto num) {
  char buffer[22] = {0}; // 2^64 is 20 digits, plus minus sign and sentinel.
  auto [ptr, ec] =
    std::to_chars(buffer, buffer + sizeof(buffer), num, /*base=*/10);
  do_concatenate_one(out, std::string_view{buffer, ptr});
}

template <typename... Args>
void do_concatenate(std::string& out, Args&&... args) {
  std::size_t required_size = sum(out.size(), estimate_string_length(args)...);
  out.reserve(required_size + 1);
  (do_concatenate_one(out, std::forward<Args>(args)), ...);
}

}
