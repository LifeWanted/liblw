#pragma once

#include <future>
#include <string_view>

#include "lw/co/future.h"
#include "lw/io/co/co.h"
#include "lw/memory/buffer.h"

namespace lw::net {

struct Address {
  std::string_view hostname;
  std::string_view service;
};

class Socket: public io::CoStream {
public:
  Socket() = default;
  Socket(Socket&& other);
  Socket& operator=(Socket&& other);
  Socket(const Socket&) = delete;
  Socket& operator=(const Socket&) = delete;
  ~Socket();

  bool is_open() const { return _socket_fd > 0; }

  /**
   * Closes the socket, disabling any further reads or writes.
   */
  void close() override;

  bool good() const override { return is_open(); }
  bool eof() const override {  return is_open(); }
  co::Future<std::size_t> write(const Buffer& data) override {
    return send(data);
  }
  co::Future<std::size_t> read(Buffer& buffer) override {
    return receive(buffer);
  }

  /**
   * Connects to the given endpoint.
   *
   * The returned future is resolved when the connection is opened.
   */
  co::Future<void> connect(Address addr);

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
  co::Future<std::size_t> send(const Buffer& data);

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
  co::Future<std::size_t> receive(Buffer& buff);

  /**
   * Binds this socket to the given address and starts listening for
   * connections. Upon successful resolution, users may `accept()` new
   * connections.
   */
  void listen(Address addr);

  /**
   * Waits for a new connection to come in.
   *
   * @return
   *  A future that will resolve to a new Socket if a connection comes in.
   */
  co::Future<Socket> accept() const;

private:
  explicit Socket(int socket_fd): _socket_fd{socket_fd} {}

  co::Future<std::size_t> _do_send(const Buffer& data, int flags);
  co::Future<std::size_t> _do_recv(Buffer& data);
  co::Future<Socket> _do_accept() const;

  int _socket_fd = 0;
};

}
