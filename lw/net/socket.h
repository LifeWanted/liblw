#pragma once

#include <future>
#include <string_view>

#include "lw/memory/buffer.h"

namespace lw::net {

struct Address {
  std::string hostname;
  std::string service;
};

class Socket {
public:
  Socket() = default;
  Socket(Socket&& other);
  Socket& operator=(Socket&& other);
  Socket(const Socket&) = delete;
  Socket& operator=(const Socket&) = delete;
  ~Socket();

  bool is_open() const { return _socket_fd > 0; }
  void close();

  /**
   * Connects to the given endpoint.
   *
   * The returned future is resolved when the connection is opened.
   */
  [[nodiscard]] std::future<void> connect(Address addr);

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

  /**
   * Binds this socket to the given address and starts listening for
   * connections. Upon successful resolution, users may `accept()` new
   * connections.
   */
  [[nodiscard]] std::future<void> listen(Address addr);

  /**
   * Waits for a new connection to come in.
   *
   * @return
   *  A future that will resolve to a new Socket if a connection comes in.
   */
  [[nodiscard]] std::future<Socket> accept() const;

private:
  explicit Socket(int socket_fd): _socket_fd{socket_fd} {}

  std::size_t do_send(const Buffer& data, int flags);

  int _socket_fd = 0;
};

}
