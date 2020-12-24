#include "lw/net/internal/errors.h"

#include <experimental/source_location>
#include <string_view>

#include "lw/err/canonical.h"
#include "lw/err/system.h"
#include "openssl/err.h"

namespace lw::net::internal {

using ::std::experimental::source_location;

void check_openssl_error(long unsigned int err_code, source_location loc) {
  if (!err_code) return;

  // OpenSSL recommends a minimum of 256 bytes for this buffer.
  char err[1024] = {0};
  ERR_error_string_n(err_code, err, sizeof(err));

  // TODO(alaina): Split OpenSSL errors by class of error instead of all being
  // Internal.
  throw Internal(loc) << "OpenSSL error: " << static_cast<char*>(err);
}

void check_openssl_error(source_location loc) {
  check_openssl_error(ERR_get_error());
}

[[noreturn]] void check_all_errors(
  std::string_view backup_message,
  source_location loc
) {
  check_openssl_error(loc);
  check_system_error(loc);
  throw Internal(loc) << backup_message;
}

}
