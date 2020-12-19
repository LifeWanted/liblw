#pragma once

#include "lw/memory/buffer.h"
#include "lw/memory/buffer_view.h"
#include "openssl/types.h"

namespace lw::net::internal {

class TLSClientImpl {
public:
  TLSClientImpl(
    SSL* client,
    BIO* encrypted,
    BIO* plaintext
  );

  ~TLSClientImpl();

  Buffer& read_buffer(std::size_t min_size);
  std::size_t buffer_encrypted_data(BufferView buffer);
  std::size_t read_decrypted_data(Buffer& buffer);

  std::size_t buffer_plaintext_data(BufferView buffer);
  Buffer read_encrypted_data(std::size_t limit);

private:
  SSL* _client = nullptr;
  BIO* _encrypted = nullptr;
  BIO* _plaintext = nullptr;
  Buffer _read_buffer;
  Buffer _write_buffer;
};

}
