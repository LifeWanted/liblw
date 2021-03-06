#pragma once

#include <atomic>
#include <memory>
#include <thread>
#include <unordered_map>
#include <utility>

#include "lw/net/router.h"
#include "lw/net/socket.h"

namespace lw::net {

class Server {
public:
  Server() = default;
  ~Server();

  Server(Server&&) = default;
  Server& operator=(Server&&) = default;

  Server(const Server&) = delete;
  Server& operator=(const Server&) = delete;

  /**
   * Add a router to the server. Any connections to the given port will be sent
   * to the router.
   */
  void attach_router(unsigned short port, Router* router);

  bool running() const { return _running; }

  /**
   * Binds sockets to listen on all the ports added to the server.
   *
   * The sockets will not receive any connections until the `run` method is
   * called.
   */
  void listen();

  /**
   * Closes the bound sockets, stopping any more connections from coming.
   *
   * Calling a `run` method after closing the connection without listening again
   * first will result in an error.
   */
  void close();

  /**
   * If no routers are currently handling any connections, closes all bound
   * sockets and returns immediately.
   *
   * @todo
   *  Create a timed version of this that allows routers to drain active
   *  connections before stopping them.
   *
   * @return
   *  True if the bound sockets were closed, otherwise false.
   */
  bool try_close();

  /**
   * Closes the bound sockets and closes all connections being handled by any
   * routers.
   *
   * This is not a graceful shutdown and should only be used when other methods
   * have failed and immediate closure is necessary.
   */
  void force_close();

  /**
   * Starts the server receiving connections on the bound sockets. This method
   * will run infinitely until the server is closed.
   *
   * This takes over running the co::Scheduler for the calling thread.
   */
  void run();

private:
  struct RouterSocket {
    Router& router;
    std::unique_ptr<Socket> socket;
    std::unique_ptr<Socket> accepting;
  };

  void _schedule_accept_loops();
  co::Future<Socket*> start_socket(unsigned short port);

  std::atomic_bool _listening = false;
  std::atomic_bool _running = false;
  std::unordered_map<unsigned short, RouterSocket> _port_map;
  std::thread::id _runner_thread;
};

}
