#include "lw/http/https.h"

#include <cstdint>
#include <experimental/source_location>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>

#include "lw/co/scheduler.h"
#include "lw/co/task.h"
#include "lw/err/canonical.h"
#include "lw/err/system.h"
#include "lw/io/co/co.h"
#include "lw/memory/buffer.h"
#include "lw/memory/buffer_view.h"
#include "openssl/err.h"
#include "openssl/ssl.h"

namespace lw {
namespace {

using std::experimental::source_location;

constexpr char* SSL_STREAM_BIO_NAME = "SSLStreamBIO";

void openssl_init() {
  static int init = ([]() {
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
    return 0;
  })();
}

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

void check_all_errors(
  std::string_view backup_message,
  source_location loc = source_location::current()
) {
  check_openssl_error(loc);
  check_system_error(loc);
  throw Internal(loc) << backup_message;
}

void context_setup_error(
  SSL_CTX* context,
  std::string_view backup_message,
  source_location loc = source_location::current()
) {
  SSL_CTX_free(context);
  check_all_errors(loc);
}

class SSLStream: public io::CoStream {
public:
  ~SSLStream() {
    if (_ssl_io) BIO_free(_ssl_io);
  }

  std::unique_ptr<SSLStream> wrap_stream(
    SSL_CTX& ssl_context,
    std::unique_ptr<io::CoStream> raw_stream
  ) {
    BIO* ssl_io = BIO_new_ssl(&ssl_context, /*client=*/false);
    if (!ssl_io) {
      check_all_errors("Unknown error instantiating SSL BIO for connection.");
    }

    BIO_METHOD* io_methods =
      BIO_meth_new(BIO_TYPE_SOURCE_SINK, SSL_STREAM_BIO_NAME);
    if (!io_methods) {
      BIO_free(ssl_io);
      check_all_errors(
        "Unknown error instantiating BIO_METHOD for connection."
      );
    }
    if (!BIO_meth_set_write(io_methods, [](BIO* io, )))

    return std::make_unique<SSLStream>(std::move(raw_stream), ssl_io);
  }

  bool eof() const override { return _raw_stream.eof(); }
  bool good() const override { return _raw_stream.good(); }

  co::Future<std::size_t> read(Buffer& buffer) override;
  co::Future<std::size_t> write(const Buffer& buffer) override;

  void close() override { return _raw_stream.close(); }

private:
  SSLStream(std::unique_ptr<io::CoStream> raw_stream, BIO* ssl_io):
    _raw_stream{std::move(raw_stream)},
    _ssl_io{ssl_io}
  {}

  std::unique_ptr<io::CoStream> _raw_stream;
  BIO* _ssl_io = nullptr;
};

}

namespace internal {

class TLSState {
public:
  ~TLSState() {
    if (_context) SSL_CTX_free(_context);
  }

  static std::unique_ptr<TLSState> from_options(
    const HttpsRouter::Options& options
  ) {
    openssl_init();

    const SSL_METHOD* method = TLS_server_method();
    SSL_CTX* context = SSL_CTX_new(method);
    if (!context) {
      check_all_errors("Unknown error instantiating SSL_CTX.");
    }
    if (!SSL_CTX_set_min_proto_version(context, TLS_1_2_VERSION)) {
      context_setup_error(context, "Unknown error setting min TLS version.");
    }

    auto key_path = std::get<std::filesystem::path>(options.private_key);
    auto cert_path = std::get<std::filesystem::path>(options.certificate);

    if (
      SSL_CTX_use_certificate_file(
        context,
        cert_path.c_str(),
        SSL_FILETYPE_PEM
      ) <= 0
    ) {
      context_setup_error(context, "Unknown error setting certificate file.");
    }

    if (
      SSL_CTX_use_PrivateKey_file(
        context,
        key_path.c_str(),
        SSL_FILETYPE_PEM
      ) <= 0
    ) {
      context_setup_error(context, "Unknown error setting private key file.");
    }

    return std::make_unique<TLSState>(context);
  }

private:
  TLSState(SSL_CTX* context): _context{context} {}

  SSL_CTX* _context = nullptr;
};

}

HttpsRouter::HttpsRouter(const Options& options):
  _tls{internal::TLSState::from_options(options)}
{}

co::Task<void> HttpsRouter::run(std::unique_ptr<io::CoStream> conn) {

}

}
