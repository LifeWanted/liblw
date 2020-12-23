#include "lw/net/tls.h"

#include <memory>

#include "lw/flags/flags.h"
#include "lw/memory/buffer.h"
#include "lw/memory/buffer_view.h"
#include "lw/net/internal/tls_client.h"
#include "lw/net/internal/tls_context.h"

LW_FLAG(
  std::size_t, tls_buffer_size, 1024 * 1024,
  "Size of write buffer for TLS streams."
);

namespace lw::net {

TLSStream::TLSStream(
  std::unique_ptr<internal::TLSClientImpl> client,
  std::unique_ptr<io::CoStream> raw_stream
):
  _client{std::move(client)},
  _raw_stream{std::move(raw_stream)},
  _write_buffer{flags::tls_buffer_size}
{}

co::Future<void> TLSStream::handshake() {
  // Get into handshake mode.
  _client->handshake();
  do {
    // Attempt to read outbound handshake data.
    internal::TLSIOResult res = _client->read_encrypted_data(_write_buffer);
    if (res && res.bytes > 0) {
      co_await _raw_stream->write(
        {_write_buffer.data(), res.bytes, /*own_data=*/false}
      );
    }

    // If handshaking needs to receive more data, read it from the stream.
    if (_client->handshake() == internal::TLSResult::NEED_TO_READ) {
      const std::size_t bytes_read = co_await _raw_stream->read(_write_buffer);
      _client->buffer_encrypted_data({_write_buffer.data(), bytes_read});
    }
  } while (_client->handshake() != internal::TLSResult::COMPLETED);
}

co::Future<std::size_t> TLSStream::read(Buffer& buffer) {
  const std::size_t bytes_read = co_await _raw_stream->read(buffer);
  internal::TLSIOResult res =
    _client->buffer_encrypted_data({buffer.data(), bytes_read});
  if (res) {
    if (res.bytes != bytes_read) {
      throw Internal() << "Failed to buffer all encrypted data from the wire.";
    }

    res = _client->read_decrypted_data(buffer);
    if (!res) {
      throw Internal() << "Failed to read decrypted data from TLS client.";
    }
    co_return res.bytes;
  }
  if (res.result == internal::TLSResult::AGAIN) co_return 0;
  throw Internal() << "Unknown failure buffering encrypted data.";
}

co::Future<std::size_t> TLSStream::write(const Buffer& buffer) {
  internal::TLSIOResult res = _client->buffer_plaintext_data(buffer);
  if (!res || res.bytes != buffer.size()) {
    throw Internal() << "Failed to buffer plaintext data into TLS client.";
  }

  // Transfer the encrypted data in a loop. The TLSStream's write buffer might
  // be smaller than buffer handed in to write.
  std::size_t bytes_transferred = 0;
  while (bytes_transferred < buffer.size()) {
    res = _client->read_encrypted_data(_write_buffer);
    if (!res) break;
    co_await _raw_stream->write(
      {_write_buffer.data(), res.bytes, /*own_data=*/false}
    );
    bytes_transferred += res.bytes;
  }

  co_return bytes_transferred;
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
