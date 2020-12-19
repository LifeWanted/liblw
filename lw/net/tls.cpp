#include "lw/net/tls.h"

#include <memory>

#include "lw/memory/buffer_view.h"
#include "lw/net/internal/tls_client.h"
#include "lw/net/internal/tls_context.h"

namespace lw::net {

TLSStream::TLSStream(
  std::unique_ptr<internal::TLSClientImpl> client,
  std::unique_ptr<io::CoStream> raw_stream
):
  _client{std::move(client)},
  _raw_stream{std::move(raw_stream)}
{}

co::Future<std::size_t> TLSStream::read(Buffer& buffer) {
  Buffer& read_buffer = _client->read_buffer(buffer.size());
  std::size_t bytes_read = co_await _raw_stream->read(read_buffer);
  std::size_t total_decrypted = 0;
  BufferView view{read_buffer.data(), bytes_read};
  Buffer out{buffer.data(), buffer.size(), /*own_data=*/false};

  while (!view.empty()) {
    std::size_t written = _client->buffer_encrypted_data(view);
    view = view.trim_prefix(written);

    std::size_t decrypted = 0;
    do {
      decrypted = _client->read_decrypted_data(out);
      out = out.trim_prefix(decrypted);
      total_decrypted += decrypted;
    } while (decrypted > 0);
  }

  co_return total_decrypted;
}

co::Future<std::size_t> TLSStream::write(const Buffer& buffer) {
  BufferView view{buffer};
  std::size_t total_written = 0;
  while (!view.empty()) {
    std::size_t written = _client->buffer_plaintext_data(view);
    view = view.trim_suffix(written);
    total_written +=
      co_await _raw_stream->write(_client->read_encrypted_data(buffer.size()));
  }

  co_return total_written;
}

TLSStreamFactory::TLSStreamFactory(const TLSOptions& options):
  _context{internal::TLSContextImpl::from_options(options)}
{}

TLSStreamFactory::~TLSStreamFactory() {
  // This destructor must be defined in this file where it has access to the
  // full definition of the TLS*Impl classes.
}

std::unique_ptr<TLSStream> TLSStreamFactory::wrap_stream(
  std::unique_ptr<io::CoStream> stream
) {
  return std::unique_ptr<TLSStream>{
    new TLSStream{_context->make_client(), std::move(stream)}
  };
}

}
