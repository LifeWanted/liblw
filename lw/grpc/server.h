#pragma once

#include <cstdint>
#include <memory>

namespace lw::grpc {
namespace internal {
class GrpcWrapper;
}

struct Options {
  std::uint16_t port;
};

class Server {
public:
  explicit Server(const Options& options);
  ~Server() = default;

  Server(Server&&) = default;
  Server& operator=(Server&&) = default;

  Server(const Server&) = delete;
  Server& operator=(const Server&) = delete;

  /**
   * Attaches all registered services and handlers.
   * @see LW_REGISTER_GRPC_HANDLER
   */
  void attach_services();

  bool running() const;

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
  std::unique_ptr<internal::GrpcWrapper> _grpc_wrapper;
};

}
