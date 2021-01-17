#pragma once

#include <functional>
#include <memory>
#include <tuple>
#include <typeindex>
#include <typeinfo>
#include <type_traits>
#include <unordered_map>

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
 * ServerResource template, specifying their dependencies as a tuple argument to
 * that class.
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

  template <typename T>
  co::Future<T*> create_and_get();

  template <typename T>
  T& unsafe_get() {
    return *_dependencies.at({typeid(T)});
  }

  template <typename T>
  const T& unsafe_get() const {
    return *_dependencies.at({typeid(T)});
  }

private:
  std::unordered_map<std::type_index, std::unique_ptr<ServerResourceBase>>
  _dependencies;
};

// -------------------------------------------------------------------------- //

namespace internal {

template <typename... Dependencies>
struct TupleFlatten;

template <typename... Unwrapped>
struct TupleFlatten<std::tuple<Unwrapped...>> {
  typedef std::tuple<std::remove_cvref_t<Unwrapped>...> flattened_types;
};

template <typename... Unwrapped, typename... NextTuple, typename... Rest>
struct TupleFlatten<
  std::tuple<Unwrapped...>,
  std::tuple<NextTuple...>,
  Rest...
>:
  public TupleFlatten<std::tuple<Unwrapped...>, NextTuple..., Rest...>
{};

template <typename... Unwrapped, typename Next, typename... Rest>
struct TupleFlatten<std::tuple<Unwrapped...>, Next, Rest...>:
  public TupleFlatten<std::tuple<Unwrapped..., Next>, Rest...>
{};

// -------------------------------------------------------------------------- //

template <typename DependencyTuple>
struct ConstructAllDeps;

template <>
struct ConstructAllDeps<std::tuple<>> {
  static void construct(ServerResourceContext& context) {}
};

template <typename Dependency, typename... Rest>
struct ConstructAllDeps<std::tuple<Dependency, Rest...>> {
  static void construct(ServerResourceContext& context) {
    context.create_and_get<Dependency>();
    ConstructAllDeps<std::tuple<Rest...>>::construct(context);
  }
};

// -------------------------------------------------------------------------- //

template <typename ServerResource, typename DependenciesTuple>
struct ServerResourceFactoryTypes;

template <typename ServerResource, typename... Dependencies>
struct ServerResourceFactoryTypes<ServerResource, std::tuple<Dependencies...>> {
  typedef ServerResource server_resource_type;
  typedef std::tuple<Dependencies...> dependency_types;
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
      typename ServerResource::dependency_types
    >::type,
    std::unique_ptr<ServerResource>
  >
{};

template <typename ServerResource, typename Factory>
class FactoryIsAsynchronous:
  public std::is_same<
    typename FactoryInvocationResult<
      Factory,
      typename ServerResource::dependency_types
    >::type,
    co::Future<std::unique_ptr<ServerResource>>
  >
{};

// -------------------------------------------------------------------------- //

template <typename ServerResource, typename DependenciesTuple>
struct FetchDependenciesAndInvoke;

template <typename ServerResource, typename... Dependencies>
struct FetchDependenciesAndInvoke<ServerResource, std::tuple<Dependencies...>> {
  template <typename Factory>
  static co::Future<std::unique_ptr<ServerResourceBase>> invoke(
    Factory& factory,
    ServerResourceContext& context
  ) {
    auto resource = co_await factory(
      (*(co_await context.create_and_get<Dependencies>()))...
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
    typename ServerResource::dependency_types
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
      typename Types::dependency_types
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
  typedef internal::TupleFlatten<std::tuple<>, Dependencies...>::flattened_types
    dependency_types;

  void initialize_dependencies() {
    internal::ConstructAllDeps<dependency_types>::construct(
      server_resource_context()
    );
  }

  template <typename T>
  T& get() {
    static_assert(
      internal::TupleContains<T, dependency_types>::value,
      "Can only fetch explicit dependency resources. Add this type to your "
      "dependency tuple in order to get it."
    );
    return server_resource_context().unsafe_get<T>();
  }

  template <typename T>
  const T& get() const {
    static_assert(
      internal::TupleContains<T, dependency_types>::value,
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
    _dependencies[idx] = co_await construct_server_resource(info, *this);
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
