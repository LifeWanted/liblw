#include "lw/net/internal/tls_client.h"

#include <memory>
#include <string_view>

#include "lw/err/canonical.h"
#include "lw/memory/buffer.h"
#include "lw/memory/buffer_view.h"
#include "lw/net/internal/errors.h"
#include "openssl/bio.h"
#include "openssl/ssl.h"

namespace lw::net::internal {

TLSClientImpl::~TLSClientImpl() {
  // BIOs are freed by the SSL client for us.
  if (_client) SSL_free(_client);
}

TLSResult TLSClientImpl::handshake() {
  if (SSL_is_init_finished(_client)) return TLSResult::COMPLETED;

  int ret = SSL_do_handshake(_client);
  int err = SSL_get_error(_client, ret);
  switch (err) {
    case SSL_ERROR_NONE:
      return TLSResult::AGAIN;
    case SSL_ERROR_WANT_READ:
      return TLSResult::NEED_TO_READ;
    case SSL_ERROR_WANT_WRITE:
      return TLSResult::NEED_TO_WRITE;
  }
  check_openssl_error(err);
  throw Internal() << "Unknown error during handshake.";
}

TLSIOResult TLSClientImpl::buffer_encrypted_data(BufferView buffer) {
  if (!BIO_write_all(_encrypted, buffer.data(), buffer.size())) {
    // TODO(alaina): Check BIO_should_retry before throwing errors if the
    // write failed. This may just be a buffer full issue and we should wait
    // for the underlying socket to become available again.
    check_all_errors("Unknown BIO write error.");
  };
  return {.result = TLSResult::COMPLETED, .bytes = buffer.size()};
}

TLSIOResult TLSClientImpl::read_decrypted_data(Buffer& buffer) {
  int bytes_read = SSL_read(_client, buffer.data(), buffer.size());
  if (bytes_read <= 0) check_all_errors("Unknown SSL read error.");
  return {
      .result = TLSResult::COMPLETED,
      .bytes = static_cast<std::size_t>(bytes_read)};
}

TLSIOResult TLSClientImpl::buffer_plaintext_data(BufferView buffer) {
  int bytes_written = SSL_write(_client, buffer.data(), buffer.size());
  if (bytes_written <= 0) {
    switch (SSL_get_error(_client, bytes_written)) {
      case SSL_ERROR_WANT_READ:
        return {.result = TLSResult::NEED_TO_READ};
      case SSL_ERROR_WANT_WRITE:
        return {.result = TLSResult::NEED_TO_WRITE};
    }

    check_all_errors("Unknown SSL write error.");
  }
  return {
      .result = TLSResult::COMPLETED,
      .bytes = static_cast<std::size_t>(bytes_written)};
}

TLSIOResult TLSClientImpl::read_encrypted_data(Buffer& buffer) {
  int bytes_read = BIO_read(_plaintext, buffer.data(), buffer.size());
  if (bytes_read < 0) {
    if (BIO_should_retry(_plaintext)) return {.result = TLSResult::AGAIN};
    check_all_errors("Unknown BIO read error.");
  }

  return {
      .result = TLSResult::COMPLETED,
      .bytes = static_cast<std::size_t>(bytes_read)};
}

} // namespace lw::net::internal
