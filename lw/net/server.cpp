#include "lw/net/server.h"

#include <chrono>
#include <exception>
#include <future>
#include <utility>
#include <vector>

#include "lw/err/canonical.h"
#include "lw/err/macros.h"
#include "lw/flags/flags.h"

LW_FLAG(
  std::chrono::duration, server_listen_deadline, std::chrono::seconds(1),
  "Maximum amount of time to wait for a socket to bind to a port and begin "
  "listening for connections."
);

namespace lw::net {
namespace {

using ::std::chrono::steady_clock;

}

Server::~Server() {
  if (!try_close()) {
    force_close();
  }
}

void Server::attach_router(unsigned short port, Router* router) {
  std::scoped_lock state_lock{_state_mutex};

  LW_CHECK_NULL(router);
  if (_listening) {
    throw FailedPrecondition()
      << "Cannot add routers to server while it is listening for connections.";
  }

  if (_port_map.count(port) > 0) {
    throw AlreadyExists()
      << "Port " << port << " is already connected to a router.";
  }
  _port_map[port].router = router;
  router->attach_routes();
}

std::future<void> Server::listen() {
  return std::async(std::launch::async, [this]() -> void {
    std::vector<std::pair<unsigned short, std::future<Socket*>>> socket_futures{
      _port_map.size()
    };
    for (auto& [port, x] : _port_map) {
      socket_futures.push_back({port, start_socket(port)});
    }

    auto deadline = steady_clock::now() + flags::server_listen_deadline;
    std::vector<std::exception_ptr> errors;

    for (auto& [port, future] : socket_futures) {
      try {
        if (future.wait_until(deadline) != std::future_status::ready) {
          throw DeadlineExceeded()
            << "Socket exceeded server_listen_deadline!";
        }

        Socket* socket = future.get();
        LW_CHECK_NULL(socket);
        _port_map[port].socket = socket;
      } catch (...) {
        errors.push_back(std::current_exception());
      }
    }

    if (errors.empty()) return;

    // We had an error, so close down all the sockets at succeeded.
    for (auto& [port, conn] : _port_map) {
      conn.socket.reset();
    }
    // TODO: Record all the exceptions that happened instead of just the first.
    std::rethrow_exception(errors.front());
  });
}

std::future<void> Server::start_socket(unsigned short port) {
  auto socket = std::make_unique<Socket>();
  
}

}
