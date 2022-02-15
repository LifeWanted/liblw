#pragma once

#include <experimental/source_location>

namespace lw {

/**
 * @brief Fetches the system-wide error code (i.e. `errno`).
 *
 * This may change the current system error. Pass the return value of this back
 * into `check_system_error` if there are any error types you are not handling.
 */
int get_system_error();

/**
 * Fetches the system-wide error code (e.g. `errno`) and checks for failure.
 *
 * @throws ::lw::Error
 *  If there is a non-zero error code stored in the system error. The exact
 *  class of exception thrown is dependent on the error code.
 */
void check_system_error(
  const std::experimental::source_location& loc =
    std::experimental::source_location::current()
);

/**
 * Checks the given error code against the system error codes.
 *
 * If available, the system error message is used.
 *
 * @throws ::lw::Error
 *  If the given error code indicates a system error. The exact class of
 *  exception thrown is dependent on the error code.
 */
void check_system_error(
  int err_code,
  const std::experimental::source_location& loc =
    std::experimental::source_location::current()
);

}
