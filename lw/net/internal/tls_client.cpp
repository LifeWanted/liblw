#include "lw/net/internal/tls_client.h"

#include <experimental/source_location>
#include <memory>
#include <string_view>

#include "lw/err/canonical.h"
#include "lw/err/system.h"
#include "lw/flags/flags.h"
#include "lw/memory/buffer.h"
#include "lw/memory/buffer_view.h"
#include "openssl/bio.h"
#include "openssl/err.h"
#include "openssl/ssl.h"

LW_FLAG(
  std::size_t, tls_buffer_size, 1024 * 1024,
  "Initial size of read and write buffers for TLS streams."
);

namespace lw::net::internal {
namespace {

using std::experimental::source_location;

void check_openssl_error(
  long unsigned int err_code,
  source_location loc = source_location::current()
) {
  if (!err_code) return;

  // OpenSSL recommends a minimum of 256 bytes for this buffer.
  char err[1024] = {0};
  ERR_error_string_n(err_code, err, sizeof(err));

  // TODO(alaina): Split OpenSSL errors by class of error instead of all being
  // Internal.
  throw Internal(loc) << "OpenSSL error: " << static_cast<char*>(err);
}

void check_openssl_error(source_location loc = source_location::current()) {
  check_openssl_error(ERR_get_error());
}

void check_all_errors(
  std::string_view backup_message,
  source_location loc = source_location::current()
) {
  check_openssl_error(loc);
  check_system_error(loc);
  throw Internal(loc) << backup_message;
}

}

TLSClientImpl::TLSClientImpl(
  SSL* client,
  BIO* encrypted,
  BIO* plaintext
):
  _client{client},
  _encrypted{encrypted},
  _plaintext{plaintext},
  _write_buffer{flags::tls_buffer_size}
{}

TLSClientImpl::~TLSClientImpl() {
  // BIOs are freed by the SSL client for us.
  if (_client) SSL_free(_client);
}

TLSResult TLSClientImpl::handshake() {
  if (SSL_is_init_finished(_client)) return TLSResult::COMPLETED;

  int ret = SSL_do_handshake(_client);
  int err = SSL_get_error(_client, ret);
  switch (err) {
    case SSL_ERROR_NONE:        return TLSResult::AGAIN;
    case SSL_ERROR_WANT_READ:   return TLSResult::NEED_TO_READ;
    case SSL_ERROR_WANT_WRITE:  return TLSResult::NEED_TO_WRITE;
  }
  check_openssl_error(err);
  throw Internal() << "Unknown error during handshake.";
}

std::size_t TLSClientImpl::buffer_encrypted_data(BufferView buffer) {
  std::size_t written = 0;
  if (!BIO_write_ex(_encrypted, buffer.data(), buffer.size(), &written)) {
    // TODO(alaina): Check BIO_should_retry before throwing errors if the
    // write failed. This may just be a buffer full issue and we should wait
    // for the underlying socket to become available again.
    check_all_errors("Unknown BIO write error.");
  };
  return written;
}

std::size_t TLSClientImpl::read_decrypted_data(Buffer& buffer) {
  std::size_t decrypted = 0;
  int ret = SSL_read_ex(_client, buffer.data(), buffer.size(), &decrypted);
  if (ret <= 0) check_all_errors("Unknown SSL read error.");
  return decrypted;
}

TLSBufferingResult TLSClientImpl::buffer_plaintext_data(BufferView buffer) {
  std::size_t written = 0;
  int ret = SSL_write_ex(_client, buffer.data(), buffer.size(), &written);
  if (ret <= 0) {
    switch (SSL_get_error(_client, ret)) {
      case SSL_ERROR_WANT_READ:   return {.result = TLSResult::NEED_TO_READ};
      case SSL_ERROR_WANT_WRITE:  return {.result = TLSResult::NEED_TO_WRITE};
    }

    check_all_errors("Unknown SSL write error.");
  }
  return {.result = TLSResult::COMPLETED, .bytes_written = written};
}

Buffer TLSClientImpl::read_encrypted_data(std::size_t limit) {
  if (limit > _write_buffer.size()) _write_buffer = Buffer{limit};

  std::size_t bytes_read = 0;
  int ret = BIO_read_ex(
    _plaintext,
    _write_buffer.data(),
    _write_buffer.size(),
    &bytes_read
  );
  if (!ret) {
    if (BIO_should_retry(_plaintext)) return Buffer{};
    check_all_errors("Unknown BIO read error.");
  }

  return Buffer{_write_buffer.data(), bytes_read, /*own_data=*/false};
}

}
