#pragma once

#include <any>
#include <coroutine>
#include <memory>
#include <tuple>

#include "lw/base/tuple.h"
#include "lw/co/concepts.h"
#include "lw/co/future.h"

namespace lw {

class Context;

namespace internal {

class [[nodiscard]] ContextScope {
public:
  explicit ContextScope(Context& context);
  ~ContextScope();

  ContextScope(ContextScope&&) = delete;
  ContextScope& operator=(ContextScope&&) = delete;
  ContextScope(const ContextScope&) = delete;
  ContextScope& operator=(const ContextScope&) = delete;

  void set_future(co::Future<void> future) {
    _future = std::make_unique<co::Future<void>>(std::move(future));
  }

  bool await_ready() const { return _future->await_ready(); }
  void await_suspend(std::coroutine_handle<> handle);
  void await_resume();

private:
  std::unique_ptr<co::Future<void>> _future;
};

}

// -------------------------------------------------------------------------- //

/**
 * @brief
 *
 * ```cpp
 *  co_await Context::current().scope([&]() -> auto {
 *    co_return some_async_function();
 *  });
 * ```
 */
class Context {
public:
  Context() = default;
  virtual ~Context() = default;

  Context(Context&&) = delete;
  Context& operator=(Context&&) = delete;
  Context(const Context&) = delete;
  Context& operator=(const Context&) = delete;

  static Context& current();

  template <co::CallableAwaitable Coro>
  internal::ContextScope scope(Coro&& coro) {
    internal::ContextScope ctx_scope{*this};
    ctx_scope.set_future(([&]() -> co::Future<void> { co_await coro(); })());
    return ctx_scope;
  }
};

class ModularContextBase : public Context {
protected:
  ModularContextBase() = default;
  ~ModularContextBase() = default;

  std::any get_module(std::size_t module_hash_code) const;

};

template <typename Dependencies>
class ModularContext;

/**
 * @brief
 *
 * @tparam Dependencies
 */
template <typename... Dependencies>
class ModularContext<std::tuple<Dependencies...>> : public ModularContextBase {
public:
  using dependency_tuple_t = sanitize_tuple_t<std::tuple<Dependencies...>>;

  template <typename Module>
  Module& get() const {
    static_assert(
      is_tuple_member_v<Module, dependency_tuple_t>,
      "Module is not a dependency of this context."
    );
    return std::any_cast<Module>(get_module(typeid(Module).hash_code()));
  }
};

}
