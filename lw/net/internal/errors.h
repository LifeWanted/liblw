#pragma once

#include <source_location>
#include <string_view>

namespace lw::net::internal {

void check_openssl_error(
    long unsigned int err_code,
    std::source_location loc = std::source_location::current());

void check_openssl_error(
    std::source_location loc = std::source_location::current());

[[noreturn]] void check_all_errors(
    std::string_view backup_message,
    std::source_location loc = std::source_location::current());

} // namespace lw::net::internal
