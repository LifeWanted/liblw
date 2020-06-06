#include "lw/net/server.h"

#include <chrono>
#include <exception>
#include <future>
#include <utility>
#include <vector>


#include <iostream>


#include "lw/co/future.h"
#include "lw/err/canonical.h"
#include "lw/err/macros.h"
#include "lw/flags/flags.h"

LW_FLAG(
  std::chrono::steady_clock::duration,
  server_listen_deadline,
  std::chrono::seconds(5),
  "Maximum amount of time to wait for sockets to bind to ports and begin "
  "listening for connections."
);

namespace lw::net {
namespace {

using ::std::chrono::steady_clock;

constexpr steady_clock::duration ACCEPT_WAIT_DURATION =
  std::chrono::microseconds(1);

}

Server::~Server() {
  if (!try_close()) {
    force_close();
  }
}

void Server::attach_router(unsigned short port, Router* router) {
  LW_CHECK_NULL(router);
  if (_listening) {
    throw FailedPrecondition()
      << "Cannot add routers to server while it is listening for connections.";
  }

  const auto [i, inserted] = _port_map.try_emplace(port, RouterSocket{*router});
  if (!inserted) {
    throw AlreadyExists()
      << "Port " << port << " is already connected to a router.";
  }

  router->attach_routes();
}

// -------------------------------------------------------------------------- //

std::future<void> Server::listen() {
  return std::async(std::launch::async, [this]() -> void {
    bool expect_listening = false;
    if (!_listening.compare_exchange_strong(expect_listening, true)) {
      throw FailedPrecondition()
        << "Server is already listening! Stop the server first.";
    }

    try {
      do_listen();
    } catch (...) {
      _listening = false;
      std::rethrow_exception(std::current_exception());
    }
  });
}

void Server::do_listen() {
  struct PendingSocket {
    unsigned short port;
    std::future<void> future;
  };

  // Start all the sockets connecting in parallel.
  std::vector<PendingSocket> pending_sockets;
  for (auto& [port, router_sock] : _port_map) {
    // Skip any already-connected ports.
    if (router_sock.socket) continue;

    router_sock.socket = std::make_unique<Socket>();
    pending_sockets.push_back({
      .port = port,
      .future = router_sock.socket->listen({
        .hostname = "localhost",
        .service = std::to_string(port)
      })
    });
  }

  // Wait up to `server_listen_deadline` for all the sockets to start
  // listening.
  auto deadline = steady_clock::now() + flags::server_listen_deadline.value();
  std::vector<std::exception_ptr> errors;
  for (auto& [port, future] : pending_sockets) {
    try {
      if (future.wait_until(deadline) != std::future_status::ready) {
        throw DeadlineExceeded()
          << "Socket exceeded server_listen_deadline!";
      }

      future.get();
    } catch (...) {
      errors.push_back(std::current_exception());
    }
  }

  if (errors.empty()) return;

  // We had an error, so close down all the sockets.
  _listening = false;
  for (auto& [port, conn] : _port_map) {
    conn.socket.reset();
  }
  // TODO: Record all the exceptions that happened instead of just the first.
  std::rethrow_exception(errors.front());
}

// -------------------------------------------------------------------------- //

bool Server::try_close() {
  // TODO: Pause accepting new connections while checking if all routers are
  // idle. Without pausing, this check is prone to race conditions.

  for (auto& [port, router] : _port_map) {
    if (router.router.connection_count() > 0) {
      return false;
    }
  }

  force_close();
  return true;
}

void Server::force_close() {
  for (auto& [port, router] : _port_map) {
    if (router.socket && router.socket->is_open()) {
      router.socket->close();
      router.socket = nullptr;
    }
  }
  _running = false;
  _listening = false;
}

// -------------------------------------------------------------------------- //

std::future<void> Server::run() {
  return std::async(std::launch::async, [this]() -> void {
    if (!_listening) {
      throw FailedPrecondition()
        << "Attach routers and start listening before attempting to run.";
    }

    bool expect_running = false;
    if (!_running.compare_exchange_strong(expect_running, true)) {
      throw FailedPrecondition()
        << "Server is already running! Stop the server first.";
    }

    try {
      if (_port_map.size() == 1) {
        do_run_one();
      } else {
        do_run();
      }
      _running = false;
    } catch (...) {
      _running = false;
      std::rethrow_exception(std::current_exception());
    }
  });
}

void Server::do_run_one() {
  auto& [port, router_sock] = *_port_map.begin();
  Router& router = router_sock.router;
  const Socket& socket = *router_sock.socket;
  while (_running) {
    std::cout << "Accepting on port " << port << std::endl;
    router.run(std::make_unique<Socket>(socket.accept().get()));
  }
}

void Server::do_run() {
  while (_running) {
    for (auto& [port, router] : _port_map) {
      if (!router.accepting.valid()) {
        router.accepting = router.socket->accept();
      }
      auto accept_status = router.accepting.wait_for(ACCEPT_WAIT_DURATION);
      if (accept_status == std::future_status::ready) {
        router.router.run(std::make_unique<Socket>(router.accepting.get()));
      }
    }
  }
}

}
