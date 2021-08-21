#pragma once

#include <functional>
#include <memory>
#include <tuple>
#include <typeindex>
#include <typeinfo>
#include <type_traits>
#include <unordered_map>

#include "lw/base/tuple.h"
#include "lw/co/future.h"

#define _LW_CONCAT_INNER(x, y) x ## y
#define _LW_CONCAT(x, y) _LW_CONCAT_INNER(x, y)

#define LW_REGISTER_RESOURCE_FACTORY(ServerResource, factory)             \
  namespace {                                                             \
  static const bool _LW_CONCAT(server_resource_registration, __LINE__) =  \
    ::lw::register_server_resource(                                       \
      typeid(ServerResource),                                             \
      ::lw::internal::create_resource_factory<ServerResource>(factory)    \
    );                                                                    \
  } // Ignore me.

// -------------------------------------------------------------------------- //

namespace lw {

class ServerResourceContext;

/**
 * Base for server resources. Final server resources should inherit from the
 * ServerResource template, specifying their dependencies in a tuple as the
 * template parameter to that class.
 */
class ServerResourceBase {
public:
  virtual ~ServerResourceBase() = default;

  /**
   * Sets the resource context that will be used to fetch/create server resource
   * dependencies.
   */
  void server_resource_context(ServerResourceContext& context) {
    _context = &context;
  }

  /**
   * Gets the resource context for dependency resources.
   */
  ServerResourceContext& server_resource_context() {
    return *_context;
  }
  const ServerResourceContext& server_resource_context() const {
    return *_context;
  }

private:
  ServerResourceContext* _context = nullptr;
};

/**
 * Context for generating and fetching server resources shared by all server
 * resources for a given request cycle.
 *
 * One of these is created for every request that comes into a router utilizing
 * server resources.
 */
class ServerResourceContext {
public:
  ServerResourceContext() = default;
  ~ServerResourceContext() = default;
  ServerResourceContext(const ServerResourceContext&) = delete;
  ServerResourceContext& operator=(const ServerResourceContext&) = delete;
  ServerResourceContext(ServerResourceContext&&) = default;
  ServerResourceContext& operator=(ServerResourceContext&&) = default;

  /**
   * Fetches the identified resource. If this is the first time the resource is
   * requested, it is constructed first.
   */
  template <typename T>
  co::Future<T*> create_and_get();

  /**
   * Fetches the identified resource without performing any check for its
   * existence first.
   */
  template <typename T>
  T& unsafe_get() {
    return *static_cast<T*>(_dependencies.at({typeid(T)}).get());
  }
  template <typename T>
  const T& unsafe_get() const {
    return *static_cast<T*>(_dependencies.at({typeid(T)}).get());
  }

private:
  std::unordered_map<std::type_index, std::unique_ptr<ServerResourceBase>>
  _dependencies;

  std::unordered_multimap<std::type_index, co::Promise<void>> _construction;
};

// -------------------------------------------------------------------------- //

namespace internal {

template <typename ServerResource, typename DependenciesTuple>
struct ServerResourceFactoryTypes;

template <typename ServerResource, typename... Dependencies>
struct ServerResourceFactoryTypes<ServerResource, std::tuple<Dependencies...>> {
  typedef ServerResource server_resource_type;
  typedef sanitize_tuple_t<std::tuple<Dependencies...>> dependencies_t;
  typedef std::function<
    co::Future<std::unique_ptr<ServerResource>>(Dependencies&...)
  > factory_type;
};

// -------------------------------------------------------------------------- //

template <typename Factory, typename DependenciesTuple>
class FactoryInvocationResult;

template <typename Factory, typename... Dependencies>
class FactoryInvocationResult<Factory, std::tuple<Dependencies...>>:
  public std::invoke_result<Factory, Dependencies&...>
{};

template <typename ServerResource, typename Factory>
class FactoryIsSynchronous:
  public std::is_same<
    typename FactoryInvocationResult<
      Factory,
      typename ServerResource::dependencies_t
    >::type,
    std::unique_ptr<ServerResource>
  >
{};

template <typename ServerResource, typename Factory>
class FactoryIsAsynchronous:
  public std::is_same<
    typename FactoryInvocationResult<
      Factory,
      typename ServerResource::dependencies_t
    >::type,
    co::Future<std::unique_ptr<ServerResource>>
  >
{};

// -------------------------------------------------------------------------- //

template <typename ServerResource, typename DependenciesTuple>
struct FetchDependenciesAndInvoke;

template <typename ServerResource>
struct FetchDependenciesAndInvoke<ServerResource, std::tuple<>> {
  template <typename Factory>
  static co::Future<std::unique_ptr<ServerResourceBase>> invoke(
    Factory& factory,
    ServerResourceContext& context
  ) {
    auto resource = co_await factory();
    resource->server_resource_context(context);
    co_return std::move(resource);
  }
};

template <typename ServerResource, typename... Dependencies>
struct FetchDependenciesAndInvoke<ServerResource, std::tuple<Dependencies...>> {
  template <typename Factory>
  static co::Future<std::unique_ptr<ServerResourceBase>> invoke(
    Factory& factory,
    ServerResourceContext& context
  ) {
    auto resource = co_await std::apply(
      [&](auto... deps) { return factory((*deps)...); },
      co_await co::all(context.create_and_get<Dependencies>()...)
    );

    resource->server_resource_context(context);
    co_return std::move(resource);
  }
};

// -------------------------------------------------------------------------- //

template <typename T, typename Tuple>
struct TupleContains: public std::false_type {};

template <typename T, typename... Rest>
struct TupleContains<T, std::tuple<T, Rest...>>: public std::true_type {};

template <typename T, typename U, typename... Rest>
struct TupleContains<T, std::tuple<U, Rest...>>:
  public TupleContains<T, std::tuple<Rest...>>
{};

// -------------------------------------------------------------------------- //

template <typename ServerResource>
class ServerResourceFactoryInvoker {
public:
  typedef ServerResourceFactoryTypes<
    ServerResource,
    typename ServerResource::dependencies_t
  > Types;

  template <typename UFactory>
  ServerResourceFactoryInvoker(UFactory&& factory) {
    if constexpr (FactoryIsSynchronous<ServerResource, UFactory>::value) {
      _factory = [factory{std::forward<UFactory>(factory)}](
        auto&... dependencies
      ) {
        return co::make_resolved_future(factory(dependencies...));
      };
    } else {
      _factory = std::forward<UFactory>(factory);
    }
  }

  co::Future<std::unique_ptr<ServerResourceBase>> operator()(
    ServerResourceContext& context
  ) {
    return internal::FetchDependenciesAndInvoke<
      typename Types::server_resource_type,
      typename Types::dependencies_t
    >::invoke(_factory, context);
  }

private:
  typename Types::factory_type _factory;
};

}

// -------------------------------------------------------------------------- //

template <typename... Dependencies>
class ServerResource: public ServerResourceBase {
public:
  typedef sanitize_tuple_t<std::tuple<Dependencies...>> dependencies_t;

protected:
  template <typename T>
  T& get() {
    static_assert(
      internal::TupleContains<T, dependencies_t>::value,
      "Can only fetch explicit dependency resources. Add this type to your "
      "dependency tuple in order to get it."
    );
    return server_resource_context().unsafe_get<T>();
  }

  template <typename T>
  const T& get() const {
    static_assert(
      internal::TupleContains<T, dependencies_t>::value,
      "Can only fetch explicit dependency resources. Add this type to your "
      "dependency tuple in order to get it."
    );
    return server_resource_context().unsafe_get<T>();
  }
};

// -------------------------------------------------------------------------- //

class ServerResourceFactoryBase {
public:
  virtual co::Future<std::unique_ptr<ServerResourceBase>> operator()(
    ServerResourceContext& context
  ) = 0;
};

template <typename ServerResource>
class ServerResourceFactory: public ServerResourceFactoryBase {
public:
  template <typename Factory>
  ServerResourceFactory(Factory&& factory):
    _factory{std::forward<Factory>(factory)}
  {}

  co::Future<std::unique_ptr<ServerResourceBase>> operator()(
    ServerResourceContext& context
  ) override {
    return _factory(context);
  }

private:
  internal::ServerResourceFactoryInvoker<ServerResource> _factory;
};

// -------------------------------------------------------------------------- //

bool register_server_resource(
  const std::type_info& resource_info,
  std::unique_ptr<ServerResourceFactoryBase> factory
);

co::Future<std::unique_ptr<ServerResourceBase>> construct_server_resource(
  const std::type_info& resource_info,
  ServerResourceContext& context
);

// -------------------------------------------------------------------------- //

template <typename T>
co::Future<T*> ServerResourceContext::create_and_get() {
  const std::type_info& info{typeid(T)};
  std::type_index idx{info};
  if (!_dependencies.contains(idx)) {
    if (_construction.contains(idx)) {
      // The dependency is currently under construction. Add ourselves to the
      // notification queue to await its completion.
      auto itr = _construction.emplace(idx, co::Promise<void>{});
      co_await itr->second.get_future();
    } else {
      // The dependency is not yet being built. Tag the type as under
      // construction, build it, and then notify everyone waiting for its
      // completion.
      _construction.emplace(idx, co::Promise<void>{});
      _dependencies[idx] = co_await construct_server_resource(info, *this);
      auto [itr, end] = _construction.equal_range(idx);
      for (; itr != end; ++itr) itr->second.set_value();
      _construction.erase(idx);
    }
  }
  co_return static_cast<T*>(_dependencies.at(idx).get());
}

// -------------------------------------------------------------------------- //

namespace internal {
template <typename ServerResource, typename Factory>
std::unique_ptr<ServerResourceFactoryBase> create_resource_factory(
  Factory&& factory
) {
  static_assert(
    FactoryIsSynchronous<ServerResource, Factory>::value ||
    FactoryIsAsynchronous<ServerResource, Factory>::value,
    "Server resource factories must return a unique_ptr to the server resource "
    "or a future unique_ptr."
  );
  return std::make_unique<ServerResourceFactory<ServerResource>>(
    std::forward<Factory>(factory)
  );
}
}

}
