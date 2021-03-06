#include "lw/flags/flags.h"

#include <algorithm>
#include <iomanip>
#include <list>
#include <ostream>
#include <string>
#include <string_view>
#include <unordered_map>

#include "lw/err/canonical.h"
#include "lw/err/macros.h"

namespace lw::cli {
namespace {

constexpr std::size_t FLAG_PRINT_WIDTH  = 80;
constexpr std::size_t MAX_FLAG_NAME     = 20;
constexpr std::string_view FLAG_PREFIX  = " --";

std::unordered_map<std::size_t, FlagBase*>& get_flag_map() {
  static std::unordered_map<std::size_t, FlagBase*>* flags =
    new std::unordered_map<std::size_t, FlagBase*>{};
  return *flags;
}

std::size_t hash_flag_name(std::string_view flag_name) {
  std::string alt_name;
  alt_name.reserve(flag_name.size());
  for (const char c : flag_name) {
    alt_name.push_back(c == '_' ? '-' : c);
  }
  return std::hash<std::string_view>()(alt_name);
}

void flags_register(FlagBase* flag) {
  LW_CHECK_NULL(flag);

  const std::size_t flag_hash = hash_flag_name(flag->name());
  if (get_flag_map().count(flag_hash) > 0) {
    throw AlreadyExists() << "Flag \"" << flag->name() << "\" already exists.";
  }
  get_flag_map().emplace(flag_hash, flag);
}

}

bool flags_exists(std::string_view flag_name) {
  return get_flag_map().count(hash_flag_name(flag_name)) > 0;
}

bool flags_cli_set(
  std::string_view flag_name,
  std::optional<std::string_view> value,
  const char* const* rest_args,
  int argc
) {
  auto flag_itr = get_flag_map().find(hash_flag_name(flag_name));
  if (flag_itr == get_flag_map().end()) {
    throw InvalidArgument() << "Flag " << flag_name << " does not exist.";
  }
  auto&& [k, flag] = *flag_itr;
  if (value) {
    flag->parse_value(*value);
  } else if (argc > 0) {
    std::string_view next_arg{*rest_args};
    if (next_arg.starts_with("--")) {
      flag->parse_value("yes");
    } else {
      flag->parse_value(next_arg);
      return true;
    }
  } else {
    flag->parse_value("yes");
  }
  return false;
}

void print_flags(std::ostream& out) {
  std::list<const FlagBase*> flags;
  std::size_t name_pad = 0;
  for (const auto& [hash, flag] : get_flag_map()) {
    flags.push_back(flag);
    name_pad = std::max(name_pad, flag->name().size());
  }
  flags.sort([](const FlagBase* a, const FlagBase* b) {
    return std::lexicographical_compare(
      a->name().begin(), a->name().end(),
      b->name().begin(), b->name().end()
    );
  });

  name_pad = std::min(name_pad + 1, MAX_FLAG_NAME);
  const auto& padding = std::string(name_pad + FLAG_PREFIX.size(), ' ');
  for (const FlagBase* flag : flags) {
    if (flag->name().size() >= MAX_FLAG_NAME) {
      out << FLAG_PREFIX << flag->name() << std::endl << padding;
    } else {
      out << FLAG_PREFIX << std::setw(name_pad) << std::left << flag->name();
    }

    // TODO(alaina): Wrap the description at FLAG_PRINT_WIDTH. Bonus points to
    // use the terminal's actual width as the wrap point.
    out
      << flag->type_name() << "; Default = " << flag->default_value_string()
      << std::endl
      << padding << flag->description() << std::endl << std::endl;
  }
}

FlagBase::FlagBase(
  std::string_view type_name,
  std::string_view name,
  std::string_view description
):
  _type_name{type_name},
  _name{name},
  _description{description}
{
  flags_register(this);
}

}
