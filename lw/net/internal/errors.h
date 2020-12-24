#pragma once

#include <experimental/source_location>
#include <string_view>

namespace lw::net::internal {

void check_openssl_error(
  long unsigned int err_code,
  std::experimental::source_location loc =
    std::experimental::source_location::current()
);

void check_openssl_error(
  std::experimental::source_location loc =
    std::experimental::source_location::current()
);

[[noreturn]] void check_all_errors(
  std::string_view backup_message,
  std::experimental::source_location loc =
    std::experimental::source_location::current()
);

}
