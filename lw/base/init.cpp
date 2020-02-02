#include "lw/base/init.h"

#include <optional>
#include <string_view>
#include <utility>

#include "lw/err/canonical.h"
#include "lw/err/macros.h"
#include "lw/flags/flags.h"

namespace lw {
namespace {

/**
 * Parses flags in the form `--name` or `--name=value`. If the value is wrapped
 * in quotes, those quotes will be removed.
 *
 * If the flag begins with "no-" and does not have `=value` suffix, then "no" is
 * used for the value.
 */
std::pair<
  std::optional<std::string_view>,
  std::optional<std::string_view>
> parse_flag(std::string_view flag) {
  // Only long flags are supported, they must start with `--`.
  if (flag.size() < 3 || !flag.starts_with("--")) {
    return {std::nullopt, std::nullopt};
  }
  std::size_t start = 2;
  std::size_t end = start + 1;
  while (end < flag.size() && flag[end] != '=') {
    ++end;
  }
  std::string_view name = flag.substr(start, end - start);
  if (end >= flag.size() - 1) {
    if (name.size() > 3 && name.starts_with("no-")) {
      // If tha name starts with "no-" and has no value, then we will assume it
      // is a boolean flag and "no" is the intended value.
      std::string_view value{name.begin(), 2};
      name.remove_prefix(3);
      return {name, value};
    }
    return {name, std::nullopt};
  }

  start = end + 1;
  end = flag.size();
  if (flag[start] == '"' || flag[start] == '\'') {
    if (flag[start] != flag[end - 1]) {
      throw InvalidArgument()
        << "Value for flag " << name << " is malformed.";
    }
    ++start;
    --end;
  }
  return {name, flag.substr(start, end - start)};
}

}

bool init(int* argc, const char** argv) {
  LW_CHECK_NULL(argc);
  LW_CHECK_NULL(argv);

  // The first argument is the program name, so skip that.
  const int original_count = *argc;
  const char** mvr = argv + 1;
  const char** back_fill = mvr;
  for (int i = 1; i < original_count; ++i, ++mvr) {
    const int remaining_count = original_count - i;
    auto&& [flag_name, opt_value] = parse_flag(*mvr);
    // TODO: Convert `_` in name to `-` in flag and vice-versa.
    if (flag_name && flags_exists(*flag_name)) {
      if (flags_cli_set(*flag_name, opt_value, mvr + 1, remaining_count - 1)) {
        // Setting the flag used the next argument, so move past it.
        --*argc;
        ++mvr;
        ++i;
      }
      --*argc;
    } else {
      // Unknown argument, move it forward in the list.
      *back_fill = *mvr;
      ++back_fill;
    }
  }

  // TODO: Add handling of `--help` flag.
  return true;
}

}
