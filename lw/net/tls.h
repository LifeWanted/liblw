#pragma once

#include <memory>

#include "lw/co/future.h"
#include "lw/io/co/co.h"
#include "lw/memory/buffer.h"
#include "lw/net/tls_options.h"

namespace lw::net {
namespace internal {

class TLSClientImpl;
class TLSContextImpl;

}

class TLSStreamFactory;

class TLSStream: public io::CoStream {
public:
  ~TLSStream();

  bool eof() const override { return _raw_stream->eof(); }
  bool good() const override { return _raw_stream->good(); }
  void close() override { return _raw_stream->close(); }

  co::Future<void> handshake();

  co::Future<std::size_t> read(Buffer& buffer) override;
  co::Future<std::size_t> write(const Buffer& buffer) override;

private:
  friend class TLSStreamFactory;

  TLSStream(
    std::unique_ptr<internal::TLSClientImpl> client,
    std::unique_ptr<io::CoStream> raw_stream
  );

  std::unique_ptr<internal::TLSClientImpl> _client;
  std::unique_ptr<io::CoStream> _raw_stream;
  Buffer _write_buffer;
};

class TLSStreamFactory {
public:
  TLSStreamFactory(const TLSOptions& options);
  ~TLSStreamFactory();

  std::unique_ptr<TLSStream> wrap_stream(std::unique_ptr<io::CoStream> stream);

private:
  std::unique_ptr<internal::TLSContextImpl> _context;
};

}
