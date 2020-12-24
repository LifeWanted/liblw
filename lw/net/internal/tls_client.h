#pragma once

#include "lw/memory/buffer.h"
#include "lw/memory/buffer_view.h"
#include "openssl/types.h"

namespace lw::net::internal {

enum class TLSResult {
  /**
   * The underlying library needs to read more data, but none is available.
   *
   * Resolve this by buffering more data using the appropriate `buffer_*_data`
   * method.
   */
  NEED_TO_READ,

  /**
   * The underlying library needs to write more data, but the buffer is full.
   *
   * Resolve this by calling `read_encrypted_data` and writing it to the wire.
   */
  NEED_TO_WRITE,

  /**
   * The requested action should be tried again later, possibly after buffering
   * more data.
   */
  AGAIN,

  /**
   * The action has completed.
   */
  COMPLETED
};

struct TLSIOResult {
  TLSResult result;
  std::size_t bytes = 0;

  operator bool() const { return result == TLSResult::COMPLETED; }
};

/**
 * Simplified interface for setting up and utilizing a TLS connection.
 *
 * This implementation uses non-blocking memory buffers for processing all
 * encryption and decryption internally. This means it is necessary to first
 * buffer decrypted or encrypted data before attempting to read. Failure to do
 * so will result in `TLSResult::NEED_TO_READ`.
 */
class TLSClientImpl {
public:
  TLSClientImpl(
    SSL* client,
    BIO* encrypted,
    BIO* plaintext
  ):
    _client{client},
    _encrypted{encrypted},
    _plaintext{plaintext}
  {}

  ~TLSClientImpl();

  /**
   * Attempts to perform a TLS handshake sequence. Safe to call repeatedly.
   *
   * A complete TLS handshake requires communicating with the other end of the
   * connection to agree to an encryption protocol and exchange keys. Roughly,
   * completing a handshake requires this sequence:
   *
   * ```
   *  while (handshake() != COMPLETED)
   *    if (read_encrypted_data(buffer))
   *      connection.write(buffer)
   *    if (handshake() == NEED_TO_READ)
   *      connection.read(buffer)
   *      buffer_encrypted_data(buffer)
   * ```
   *
   * @return
   *  The desired next step for the handshake to finish. If the handshake is
   *  finished, `TLSResult::COMPLETED` is returned.
   */
  TLSResult handshake();

  /**
   * Add encrypted data to the decryption buffer.
   *
   * Data buffered with this method may be read as plaintext using the
   * `read_decrypted_data` method.
   *
   * @retrun The number of bytes written to the decryption buffer.
   */
  TLSIOResult buffer_encrypted_data(BufferView buffer);

  /**
   * Reads as plaintext data which was buffered using `buffer_encrypted_data`.
   */
  TLSIOResult read_decrypted_data(Buffer& buffer);

  /**
   * Add plaintext data to the encryption buffer.
   *
   * Data buffered with this method may be read encrypted using the
   * `read_encrypted_data` method.
   *
   * @return The number of bytes written to the encryption buffer.
   */
  TLSIOResult buffer_plaintext_data(BufferView buffer);

  /**
   * Reads encrypted data previously buffered using `buffer_plaintext_data`.
   */
  TLSIOResult read_encrypted_data(Buffer& buffer);

private:
  SSL* _client = nullptr;
  BIO* _encrypted = nullptr;
  BIO* _plaintext = nullptr;
};

}
