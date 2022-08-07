#pragma once

#include <memory>
#include <tuple>

#include "lw/base/tuple.h"
#include "lw/co/future.h"
#include "lw/http/http_handler.h"
#include "lw/net/router.h"
#include "lw/net/server_resource.h"

namespace lw {
namespace internal {

template <typename DependenciesTuple>
struct RestDeps;

template <typename... Dependencies>
struct RestDeps<std::tuple<Dependencies...>> {
  static co::Future<void> initialize(net::ServerResourceContext& context) {
    co_await co::all(context.create_and_get<Dependencies>()...);
  }
};

}

class BaseRestHandler {
public:
  BaseRestHandler() = default;
  virtual ~BaseRestHandler() = default;

  void set_request_response(
    const HttpRequest& request,
    HttpResponse& response
  ) {
    _request = RestRequest{request};
    _response = RestResponse{request, response};
  }

  net::ServerResourceContext& resource_context() { return _resource_context; }
  const net::ServerResourceContext& resource_context() const {
    return _resource_context;
  }

  const RestRequest& request() const { return _request; }
  RestResponse& response() { return _response; }
  const RestResponse& response() const { return _response; }

  virtual co::Future<void> pre_method() {
    return co::make_resolved_future<void>();
  }
  virtual co::Future<void> post_method() {
    return co::make_resolved_future<void>();
  }

  virtual co::Future<void> del() {      return _default_behavior(); }
  virtual co::Future<void> get() {      return _default_behavior(); }
  virtual co::Future<void> head() {     return _default_behavior(); }
  virtual co::Future<void> options() {  return _default_behavior(); }
  virtual co::Future<void> patch() {    return _default_behavior(); }
  virtual co::Future<void> post() {     return _default_behavior(); }
  virtual co::Future<void> put() {      return _default_behavior(); }

private:
  co::Future<void> _default_behavior();

  net::ServerResourceContext _resource_context;
  RestRequest _request;
  RestResponse _response;
};

template <typename Dependencies>
class RestHandler;

template <typename... Dependencies>
class RestHandler<std::tuple<Dependencies...>>: public BaseRestHandler {
public:
  typedef sanitize_tuple_t<std::tuple<Dependencies>> dependencies_t;

  template <typename Dependency>
  Dependency& get() {
    static_assert(
      is_tuple_member_v<Dependency, dependencies_t>,
      "Can only get server resources which are explicitly listed as direct "
      "dependencies of this handler."
    );
    return resource_context().unsafe_get<Dependency>();
  }

  template <typename Dependency>
  const Dependency& get() const {
    static_assert(
      is_tuple_member_v<Dependency, dependencies_t>,
      "Can only get server resources which are explicitly listed as direct "
      "dependencies of this handler."
    );
    return resource_context().unsafe_get<Dependency>();
  }
};

class BaseRestHandlerFactory {
public:
  explicit BaseRestHandlerFactory(std::string_view route): _route{route} {}
  virtual ~BaseRestHandlerFactory() = default;

  std::string_view route() const { return _route; }

  virtual co::Future<std::unique_ptr<BaseRestHandler>> make_handler(
    const HttpRequest& request,
    HttpResponse& response
  ) const = 0;

private:
  std::string _route;
};

template <typename HandlerType>
class RestHandlerFactory: public BaseRestHandlerFactory {
public:
  typedef HandlerType handler_t;
  typedef std::unique_ptr<handler_t> handler_ptr_t;

  explicit RestHandlerFactory(std::string_view route):
    BaseRestHandlerFactory{route}
  {}

  co::Future<std::unique_ptr<BaseRestHandler>> make_handler(
    const HttpRequest& request,
    HttpResponse& response
  ) const override {
    auto handler = std::make_unique<HandlerType>();
    co_await internal::RestDeps<
      typename HandlerType::dependencies_t
    >::initialize(handler->resource_context());
    handler->set_request_response(request, response);
    co_return handler;
  }
};

}
