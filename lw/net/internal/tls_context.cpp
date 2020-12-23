#include "lw/net/internal/tls_context.h"

#include <experimental/source_location>
#include <memory>
#include <string_view>

#include "lw/err/canonical.h"
#include "lw/err/system.h"
#include "openssl/bio.h"
#include "openssl/crypto.h"
#include "openssl/err.h"
#include "openssl/ssl.h"

namespace lw::net::internal {
namespace {

using std::experimental::source_location;

void check_openssl_error(source_location loc = source_location::current()) {
  auto err_code = ERR_get_error();
  if (!err_code) return;

  // OpenSSL recommends a minimum of 256 bytes for this buffer.
  char err[1024] = {0};
  ERR_error_string_n(err_code, err, sizeof(err));

  // TODO(alaina): Split OpenSSL errors by class of error instead of all being
  // Internal.
  throw Internal(loc) << "OpenSSL error: " << static_cast<char*>(err);
}

[[noreturn]] void check_all_errors(
  std::string_view backup_message,
  source_location loc = source_location::current()
) {
  check_openssl_error(loc);
  check_system_error(loc);
  throw Internal(loc) << backup_message;
}

[[noreturn]] void context_setup_error(
  SSL_CTX* context,
  std::string_view backup_message,
  source_location loc = source_location::current()
) {
  SSL_CTX_free(context);
  check_all_errors(backup_message, loc);
}

}

TLSContextImpl::~TLSContextImpl() {
  if (_context) SSL_CTX_free(_context);
}

std::unique_ptr<TLSContextImpl> TLSContextImpl::from_options(
  const TLSOptions& options
) {
  // Instantiate the SSL context.
  SSL_CTX* context = SSL_CTX_new(
    options.connection_mode == TLSOptions::ACCEPT
      ? TLS_server_method()
      : TLS_client_method()
  );
  if (!context) {
    check_all_errors("Unknown error instantiating SSL_CTX.");
  }
  if (!SSL_CTX_set_min_proto_version(context, TLS1_2_VERSION)) {
    context_setup_error(context, "Unknown error setting min TLS version.");
  }

  // Configure the certificate and private key.
  auto key_path = std::get<std::filesystem::path>(options.private_key);
  auto cert_path = std::get<std::filesystem::path>(options.certificate);
  if (
    SSL_CTX_use_certificate_file(
      context,
      cert_path.c_str(),
      SSL_FILETYPE_PEM
    ) != 1
  ) {
    context_setup_error(context, "Unknown error setting certificate file.");
  }
  if (
    SSL_CTX_use_PrivateKey_file(
      context,
      key_path.c_str(),
      SSL_FILETYPE_PEM
    ) != 1
  ) {
    context_setup_error(context, "Unknown error setting private key file.");
  }
  if (SSL_CTX_check_private_key(context) != 1) {
    context_setup_error(context, "Unknown error with private key.");
  }

  return std::unique_ptr<TLSContextImpl>{new TLSContextImpl{options, context}};
}

std::unique_ptr<TLSClientImpl> TLSContextImpl::make_client() {
  BIO* encrypted = BIO_new(BIO_s_mem());
  if (!encrypted) {
    check_all_errors("Unknown error instantiating encrypted BIO.");
  }
  BIO* plaintext = BIO_new(BIO_s_mem());
  if (!plaintext) {
    BIO_free(encrypted);
    check_all_errors("Unknown error instantiating plaintext BIO.");
  }

  SSL* client = SSL_new(_context);
  if (!client) {
    BIO_free(encrypted);
    BIO_free(plaintext);
    check_all_errors("Unknown error creating SSL client.");
  }

  if (_connection_mode == TLSOptions::ACCEPT) {
    SSL_set_accept_state(client);
  } else {
    SSL_set_connect_state(client);
  }
  SSL_set_bio(client, encrypted, plaintext);
  return std::make_unique<TLSClientImpl>(client, encrypted, plaintext);
}

}
