#pragma once

#include <future>
#include <string_view>

#include "lw/memory/buffer.h"

namespace lw::net {

struct Address {
  std::string_view hostname;
  std::string_view service;
};

class Socket {
public:
  Socket() = default;
  Socket(Socket&& other);
  Socket(const Socket&) = delete;
  ~Socket();

  bool is_open() const { return _socket_fd > 0; }
  void close();

  /**
   * Connects to the given endpoint.
   *
   * The returned future is resolved when the connection is opened.
   */
  [[nodiscard]] std::future<void> connect(const Address& addr);

  /**
   * Sends the given buffer over the socket.
   *
   * @param data
   *  A buffer containing the information to be sent. The memory referenced by
   *  the buffer must be retained until the returned future resolves.
   *
   * @return
   *  A future that will resolve to the number of bytes sent on the wire.
   */
  [[nodiscard]] std::future<std::size_t> send(const Buffer& data);

  /**
   * Reads up to `buff->size()` bytes from the socket.
   *
   * @param buff
   *  A memory buffer to write data to. The memory referenced by the buffer must
   *  be retained until the returned future resolves.
   *
   * @return
   *  A future that will resolve to the number of bytes read from the wire.
   */
  [[nodiscard]] std::future<std::size_t> receive(Buffer* buff);

private:
  std::size_t do_send(const Buffer& data, int flags);

  int _socket_fd = 0;
};

}
