#include "lw/flags/flags.h"

#include <string>
#include <string_view>
#include <unordered_map>

#include "lw/err/canonical.h"
#include "lw/err/macros.h"

namespace lw::cli {
namespace {

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
