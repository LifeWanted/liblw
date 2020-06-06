#include "lw/net/socket.h"

#include <cerrno>
#include <experimental/source_location>
#include <future>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>


#include <iostream>


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

std::future<void> Socket::connect(Address addr) {
  return std::async(std::launch::async, [this, addr{std::move(addr)}]() {
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
      int sock = ::socket(mvr->ai_family, mvr->ai_socktype, mvr->ai_protocol);
      if (sock <= 0) continue;

      if (::connect(sock, mvr->ai_addr, mvr->ai_addrlen) == -1) {
        ::close(sock);
        continue;
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
  });
}

std::future<std::size_t> Socket::send(const Buffer& data) {
  return std::async(std::launch::async, [&]() -> std::size_t {
    if (!is_open()) {
      throw FailedPrecondition() << "Socket is not open before sending.";
    }
    LW_CHECK_NULL(data.data());

    return do_send(data, /*flags=*/0);
  });
}

std::size_t Socket::do_send(const Buffer& data, int flags) {
  int bytes_sent = ::send(_socket_fd, data.data(), data.size(), flags);
  if (bytes_sent > 0) return static_cast<std::size_t>(bytes_sent);

  // The EMSGSIZE error indicates that the message was too large. We can try to
  // split the message into smaller parts and send again.
  if (bytes_sent == EMSGSIZE) {
    if (data.size() < 2) {
      throw ResourceExhausted()
        << "Message too large to send but too small to split.";
    }
    std::size_t half_size = data.size() / 2;
    return (
      do_send(data.trim_suffix(data.size() - half_size), flags) +
      do_send(data.trim_prefix(half_size), flags)
    );
  }

  // Some other error happened, so fail out!
  check_system_error();
  throw Internal()
    << "Unknown socket error while sending " << data.size() << " bytes.";
}

std::future<std::size_t> Socket::receive(Buffer* buff) {
  return std::async(std::launch::async, [this, buff]() -> std::size_t {
    if (!is_open()) {
      throw FailedPrecondition() << "Socket is not open before receiving.";
    }
    LW_CHECK_NULL(buff);
    LW_CHECK_NULL(buff->data());

    int bytes_received =
      ::recv(_socket_fd, (void*)buff->data(), buff->size(), /*flags=*/0);

    if (bytes_received < 0) {
      check_system_error();
      throw Internal()
        << "Unknown socket error while receiving.";
    }
    if (bytes_received == 0) {
      this->close();
    }
    return bytes_received;
  });
}

std::future<void> Socket::listen(Address addr) {
  return std::async(std::launch::async, [this, addr{std::move(addr)}]() {
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
      int sock = ::socket(mvr->ai_family, mvr->ai_socktype, mvr->ai_protocol);
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
  });
}

std::future<Socket> Socket::accept() const {
  return std::async(std::launch::async, [this]() -> Socket {
    if (!is_open()) {
      throw FailedPrecondition()
        << "Socket is not open before accepting new connections.";
    }

    std::cout << "Accepting on " << _socket_fd << std::endl;

    ::sockaddr_storage remote_addr;
    socklen_t socket_size = sizeof(remote_addr);
    int new_sock = ::accept(
      _socket_fd,
      reinterpret_cast<::sockaddr*>(&remote_addr),
      &socket_size
    );

    if (new_sock < 0) {
      check_system_error();
      throw Internal() << "Unknown system error in accept.";
    }

    return Socket{new_sock};
  });
}

}
