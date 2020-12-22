#include "lw/net/internal/tls_client.h"

#include <memory>
#include <string_view>
#include <variant>

#include "gtest/gtest.h"
#include "lw/memory/buffer.h"
#include "lw/memory/buffer_view.h"
#include "lw/net/internal/tls_context.h"
#include "lw/net/tls_options.h"
#include "lw/net/testing/tls_credentials.h"

namespace lw::net::internal {
namespace {

TEST(TLSClientImpl, CanCommunicateWithItself) {
  std::unique_ptr<TLSContextImpl> client_ctx = TLSContextImpl::from_options({
    .private_key = {testing::KEY_PATH},
    .certificate = {testing::CERT_PATH},
    .connection_mode = TLSOptions::CONNECT
  });

  std::unique_ptr<TLSContextImpl> server_ctx = TLSContextImpl::from_options({
    .private_key = {testing::KEY_PATH},
    .certificate = {testing::CERT_PATH},
    .connection_mode = TLSOptions::ACCEPT
  });

  std::unique_ptr<TLSClientImpl> client_client = client_ctx->make_client();
  std::unique_ptr<TLSClientImpl> server_client = server_ctx->make_client();

  // Introduce eachother.
  TLSResult client_handshake = client_client->handshake();
  TLSResult server_handshake = server_client->handshake();
  do {
    client_handshake = client_client->handshake();
    Buffer data = client_client->read_encrypted_data(1024);
    if (!data.empty()) {
      ASSERT_EQ(server_client->buffer_encrypted_data(data), data.size());
    }
    client_handshake = client_client->handshake();

    server_handshake = server_client->handshake();
    data = server_client->read_encrypted_data(1024);
    if (!data.empty()) {
      ASSERT_EQ(client_client->buffer_encrypted_data(data), data.size());
    }
    server_handshake = server_client->handshake();
  } while (
    client_handshake != TLSResult::COMPLETED ||
    server_handshake != TLSResult::COMPLETED
  );

  // Write client plaintext data.
  const std::string_view message = "my secret message, do not steal";
  const Buffer message_buffer{message.begin(), message.end()};
  TLSBufferingResult res = client_client->buffer_plaintext_data(message_buffer);
  EXPECT_TRUE(res);
  EXPECT_EQ(res.bytes_written, message_buffer.size());

  // Transfer the encrypted data across the "wire."
  Buffer wire_buffer;
  wire_buffer = client_client->read_encrypted_data(1024);
  EXPECT_GE(wire_buffer.size(), message_buffer.size());
  EXPECT_EQ(
    server_client->buffer_encrypted_data(wire_buffer),
    wire_buffer.size()
  );

  // Read plaintext data out of the server.
  Buffer server_read_buffer{1024};
  const std::size_t bytes_decrypted =
    server_client->read_decrypted_data(server_read_buffer);
  EXPECT_EQ(bytes_decrypted, message.size());

  const BufferView server_read_view{server_read_buffer.data(), bytes_decrypted};
  EXPECT_EQ(static_cast<std::string_view>(server_read_view), message);
}

}
}
