#include "lw/net/socket.h"

#include <cerrno>
#include <experimental/source_location>
#include <fcntl.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "lw/co/scheduler.h"
#include "lw/err/canonical.h"
#include "lw/err/macros.h"
#include "lw/err/system.h"
#include "lw/flags/flags.h"

LW_FLAG(
  int, connection_backlog_limit, 10,
  "Maximum pending connections in socket accept queue."
);

namespace lw::net {
namespace {

using std::experimental::source_location;

void check_gai_error(
  int err,
  const source_location& loc = source_location::current()
) {
  switch(err) {
    case 0: {
      return;
    }

    case EAI_BADFLAGS:
    case EAI_MEMORY: {
      throw Internal(loc) << "getaddrinfo failed: " << ::gai_strerror(err);
    }

    case EAI_NONAME:
    case EAI_SOCKTYPE: {
      throw InvalidArgument(loc)
        << "getaddrinfo failed: " << ::gai_strerror(err);
    }

    case EAI_ADDRFAMILY:
    case EAI_NODATA:
    case EAI_SERVICE: {
      throw NotFound(loc) << "getaddrinfo failed: " << ::gai_strerror(err);
    }

    case EAI_AGAIN: // TODO: implement retrying?
    case EAI_FAIL: {
      throw Unavailable(loc) << "getaddrinfo failed: " << ::gai_strerror(err);
    }

    case EAI_FAMILY: {
      throw Unimplemented(loc) << "getaddrinfo failed: " << ::gai_strerror(err);
    }

    case EAI_SYSTEM: {
      check_system_error();
      throw Internal()
        << "getaddrinfo indicated a system error, but no system error found.";
    }

    default: {
      throw Internal() // No "loc", this is an error with this method.
        << "Unknown getaddrinfo failure code :" << err;
    }
  }
}

void enable_socket_reuse(int sock) {
  int set_true = 1;
  if (
    ::setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &set_true, sizeof(set_true)) ==
    -1
  ) {
    check_system_error();
    throw Internal() << "Unknown system error in setsockopt.";
  }
}

bool should_wait(int err) {
  // These errors indicate we need to wait for the socket to be ready before
  // trying again.
  return err == EAGAIN || err == EWOULDBLOCK;
}

}

Socket::Socket(Socket&& other): _socket_fd{other._socket_fd} {
  other._socket_fd = 0;
}

Socket& Socket::operator=(Socket&& other) {
  if (is_open()) close();
  _socket_fd = other._socket_fd;
  other._socket_fd = 0;
  return *this;
}

Socket::~Socket() {
  if (_socket_fd) {
    this->close(); // Disambiguate from ::close(int).
  }
}

void Socket::close() {
  if (_socket_fd == 0) {
    throw FailedPrecondition()
      << "Socket is already closed, cannot close again.";
  }
  ::close(_socket_fd);
  _socket_fd = 0;
}

co::Future<void> Socket::connect(Address addr) {
  if (is_open()) {
    throw FailedPrecondition() << "Socket is already open before connecting.";
  }

  ::addrinfo* addresses;
  ::addrinfo hints{
    .ai_family = AF_UNSPEC,
    .ai_socktype = SOCK_STREAM
  };

  int err = ::getaddrinfo(
    addr.hostname.data(),
    addr.service.data(),
    &hints,
    &addresses
  );
  check_gai_error(err);

  ::addrinfo* mvr = addresses;
  for (; mvr != nullptr; mvr = mvr->ai_next) {
    int sock = ::socket(
      mvr->ai_family,
      mvr->ai_socktype | SOCK_NONBLOCK,
      mvr->ai_protocol
    );
    if (sock <= 0) continue;

    int conn_result = ::connect(sock, mvr->ai_addr, mvr->ai_addrlen);
    if (conn_result == -1) {
      if (errno != EINPROGRESS) {
        ::close(sock);
        continue;
      }

      co_await co::fd_writable(sock);
      int err = 0;
      ::socklen_t optlen = sizeof(err);
      if (::getsockopt(sock, SOL_SOCKET, SO_ERROR, &err, &optlen) != 0) {
        check_system_error();
        throw Internal() << "Unknown error retrieving socket options.";
      }
      if (err != 0) {
        ::close(sock);
        continue;
      }
    }

    _socket_fd = sock;
    break;
  }
  ::freeaddrinfo(addresses);

  if (_socket_fd == 0) {
    // TODO: Include error message from the failed socket or connect call.
    throw Internal()
      << "Failed to connect to " << addr.hostname << ':' << addr.service;
  }
}

co::Future<std::size_t> Socket::send(const Buffer& data) {
  if (!is_open()) {
    throw FailedPrecondition() << "Socket is not open before sending.";
  }
  if (data.empty()) {
    throw InvalidArgument() << "Must send at lest 1 byte of data.";
  }
  LW_CHECK_NULL(data.data());

  return _do_send(data, /*flags=*/0);
}

co::Future<std::size_t> Socket::_do_send(const Buffer& data, int flags) {
  int bytes_sent = ::send(_socket_fd, data.data(), data.size(), flags);
  if (bytes_sent > 0) co_return static_cast<std::size_t>(bytes_sent);

  if (should_wait(errno)) {
    co_await co::fd_writable(_socket_fd);
    std::size_t sent = co_await _do_send(data, flags);
    co_return sent;
  }

  // The EMSGSIZE error indicates that the message was too large. We can try to
  // split the message into smaller parts and send again.
  if (errno == EMSGSIZE) {
    if (data.size() < 2) {
      throw ResourceExhausted()
        << "Message too large to send but too small to split.";
    }
    std::size_t half_size = data.size() / 2;
    std::size_t first_send =
      co_await _do_send(data.trim_suffix(data.size() - half_size), flags);
    std::size_t second_send =
      co_await _do_send(data.trim_prefix(half_size), flags);
    co_return first_send + second_send;
  }

  // Some other error happened, so fail out!
  check_system_error();
  throw Internal()
    << "Unknown socket error while sending " << data.size() << " bytes.";
}

co::Future<std::size_t> Socket::receive(Buffer& buff) {
  if (!is_open()) {
    throw FailedPrecondition() << "Socket is not open before receiving.";
  }
  if (buff.empty()) {
    throw InvalidArgument() << "Buffer must have capacity for at least 1 byte.";
  }
  LW_CHECK_NULL(buff.data());

  return _do_recv(buff);
}

co::Future<std::size_t> Socket::_do_recv(Buffer& buff) {
  int bytes_received =
    ::recv(_socket_fd, (void*)buff.data(), buff.size(), /*flags=*/0);
  if (bytes_received > 0) co_return static_cast<std::size_t>(bytes_received);
  if (bytes_received == 0) {
    // Receiving 0 bytes means our peer closed up.
    this->close();
    co_return static_cast<std::size_t>(bytes_received);
  }

  // Wait for more data to arrive on the socket before trying again.
  if (should_wait(errno)) {
    co_await co::fd_readable(_socket_fd);
    std::size_t received = co_await _do_recv(buff);
    co_return received;
  }

  // Some kind of error occurred.
  check_system_error();
  throw Internal()
    << "Unknown socket error while receiving.";
}

void Socket::listen(Address addr) {
  if (is_open()) {
    throw FailedPrecondition() << "Socket is already open before listening.";
  }

  ::addrinfo* addresses;
  ::addrinfo hints{
    .ai_flags = AI_PASSIVE,
    .ai_family = AF_UNSPEC,
    .ai_socktype = SOCK_STREAM
  };

  int err = ::getaddrinfo(
    addr.hostname.size() ? addr.hostname.data() : nullptr,
    addr.service.data(),
    &hints,
    &addresses
  );
  check_gai_error(err);

  ::addrinfo* mvr = addresses;
  for (; mvr != nullptr; mvr = mvr->ai_next) {
    int sock = ::socket(
      mvr->ai_family,
      mvr->ai_socktype | SOCK_NONBLOCK,
      mvr->ai_protocol);
    if (sock <= 0) continue;

    enable_socket_reuse(sock);
    if (::bind(sock, mvr->ai_addr, mvr->ai_addrlen) == -1) {
      ::close(sock);
      continue;
    }

    _socket_fd = sock;
    break;
  }
  ::freeaddrinfo(addresses);

  if (!is_open()) {
    throw Unavailable()
      << "Unable to bind to address \"" << addr.hostname << "\" for service "
      << addr.service;
  }

  if (::listen(_socket_fd, flags::connection_backlog_limit) == -1) {
    check_system_error();
    throw Internal() << "Unknown system error in listen.";
  }
}

co::Future<Socket> Socket::accept() const {
  if (!is_open()) {
    throw FailedPrecondition()
      << "Socket is not open before accepting new connections.";
  }

  return _do_accept();
}

co::Future<Socket> Socket::_do_accept() const {
  ::sockaddr_storage remote_addr;
  socklen_t socket_size = sizeof(remote_addr);
  int new_sock = ::accept(
    _socket_fd,
    reinterpret_cast<::sockaddr*>(&remote_addr),
    &socket_size
  );

  if (new_sock > 0) {
    int flags = ::fcntl(new_sock, F_GETFL);
    if (flags == -1) {
      check_system_error();
      throw Internal() << "Unknown system error getting socket flags.";
    }
    if (::fcntl(new_sock, F_SETFL, flags | O_NONBLOCK) == -1) {
      check_system_error();
      throw Internal() << "Unknown system error setting socket to nonblocking.";
    }

    co_return Socket{new_sock};
  }

  if (should_wait(errno)) {
    co_await co::fd_readable(_socket_fd);
    Socket sock = co_await _do_accept();
    co_return sock;
  }

  check_system_error();
  throw Internal() << "Unknown system error in accept.";
}

}
