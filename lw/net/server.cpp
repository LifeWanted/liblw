#include "lw/net/server.h"

#include <exception>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "lw/co/scheduler.h"
#include "lw/co/task.h"
#include "lw/err/canonical.h"
#include "lw/err/macros.h"

namespace lw::net {
namespace {

co::Task<void> accept_loop(std::atomic_bool* running, Router* router, Socket* socket) {
  while (*running) {
    Socket client = co_await socket->accept();
    co::Scheduler::this_thread().schedule(
      router->run(std::make_unique<Socket>(std::move(client)))
    );
  }
}

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

void Server::listen() {
  bool expect_listening = false;
  if (!_listening.compare_exchange_strong(expect_listening, true)) {
    throw FailedPrecondition()
      << "Server is already listening! Stop the server first.";
  }

  try {
    for (auto& [port, router_sock] : _port_map) {
      // Skip any already-connected ports.
      if (!router_sock.socket) router_sock.socket = std::make_unique<Socket>();
      if (router_sock.socket->is_open()) continue;

      router_sock.socket->listen({
        .hostname = "localhost",
        .service = std::to_string(port)
      });
    }
  } catch (...) {
    for (auto& [port, router_sock] : _port_map) {
      if (router_sock.socket) router_sock.socket = nullptr;
    }

    _listening = false;
    std::rethrow_exception(std::current_exception());
  }
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
  bool expected_running = true;
  if (!_running.compare_exchange_strong(expected_running, false)) return;

  _listening = false;
  for (auto& [port, router] : _port_map) {
    if (router.socket && router.socket->is_open()) {
      router.socket->close();
      router.socket = nullptr;
    }
  }
  co::Scheduler::for_thread(_runner_thread).stop();
}

// -------------------------------------------------------------------------- //

void Server::run() {
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
    _runner_thread = std::this_thread::get_id();
    _schedule_accept_loops();
    co::Scheduler::this_thread().run();
    _running = false;
  } catch (...) {
    _running = false;
    std::rethrow_exception(std::current_exception());
  }
}

void Server::_schedule_accept_loops() {
  for (auto& [port, router_sock] : _port_map) {
    co::Scheduler::this_thread().schedule(
      accept_loop(&_running, &router_sock.router, router_sock.socket.get())
    );
  }
}

}
