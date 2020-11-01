#pragma once

#include <future>
#include <memory>
#include <unordered_set>

#include "lw/co/task.h"
#include "lw/io/co/co.h"

namespace lw::net {

class HandlerBase {
public:
  HandlerBase() = default;
  virtual ~HandlerBase() = default;
};

class RouteBase {
public:
  RouteBase() = default;
  virtual ~RouteBase() = default;
};

/**
 * Registers the given route for a router.
 *
 * @param router_id
 *  The `typeid` hash code for the router which will service this route.
 *
 * @param route
 *  The route to register.
 */
void register_route(std::size_t router_id, RouteBase* route);

template <typename Router>
void register_route(RouteBase* route) {
  register_route(typeid(Router).hash_code(), route);
}

class Router {
public:
  Router() = default;
  virtual ~Router() = default;

  virtual void attach_routes() = 0;
  virtual co::Task<void> run(std::unique_ptr<io::CoStream> conn) = 0;
  virtual std::size_t connection_count() const = 0;

protected:
  const std::unordered_set<RouteBase*>& get_registered_routes() const;
};

}
