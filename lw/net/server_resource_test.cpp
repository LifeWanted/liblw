#include "lw/net/server_resource.h"

#include <memory>
#include <typeinfo>

#include "gtest/gtest.h"
#include "lw/co/future.h"
#include "lw/co/scheduler.h"
#include "lw/co/task.h"
#include "lw/co/testing/destroy_scheduler.h"

namespace lw {

template <typename T>
struct TestServerResource: public ServerResource<> {
  template <typename U>
  TestServerResource(U&& val): value{std::forward<U>(val)} {}

  T value;
};

LW_REGISTER_RESOURCE_FACTORY(TestServerResource<int>, []() {
  static int counter = 0;
  return std::make_unique<TestServerResource<int>>(++counter);
});

template <typename T>
struct DependentTestServerResource:
  public ServerResource<std::tuple<TestServerResource<T>>>
{
  template <typename U>
  DependentTestServerResource(U&& val): value{std::forward<U>(val)} {}

  T value;
};

LW_REGISTER_RESOURCE_FACTORY(
  DependentTestServerResource<int>,
  [](TestServerResource<int>& dep) {
    return std::make_unique<DependentTestServerResource<int>>(dep.value);
  }
);

struct OtherDependentTestServerResource:
  public ServerResource<std::tuple<TestServerResource<int>>>
{
  OtherDependentTestServerResource(int val): value{val} {}

  int value;
};

LW_REGISTER_RESOURCE_FACTORY(
  OtherDependentTestServerResource,
  [](TestServerResource<int>& dep) {
    return std::make_unique<OtherDependentTestServerResource>(dep.value);
  }
);

struct DependencyConvergenceTestServerResource:
  public ServerResource<
    std::tuple<
      DependentTestServerResource<int>,
      OtherDependentTestServerResource
    >
  >
{
  DependencyConvergenceTestServerResource(int val): value{val} {}

  int value;
};

LW_REGISTER_RESOURCE_FACTORY(
  DependencyConvergenceTestServerResource,
  [](DependentTestServerResource<int>& a, OtherDependentTestServerResource& b) {
    return std::make_unique<DependencyConvergenceTestServerResource>(
      a.value * b.value
    );
  }
);

template <typename T>
struct AsyncTestServerResource: public ServerResource<> {
  template <typename U>
  AsyncTestServerResource(U&& val): value{std::forward<U>(val)} {}

  T value;
};

LW_REGISTER_RESOURCE_FACTORY(
  AsyncTestServerResource<int>,
  []() -> co::Future<std::unique_ptr<AsyncTestServerResource<int>>> {
    static int counter = 0;
    co_await co::next_tick();
    co_return std::make_unique<AsyncTestServerResource<int>>(++counter);
  }
);

// -------------------------------------------------------------------------- //

namespace {
template <typename Func>
void run(Func&& func) {
  co::Scheduler::this_thread().schedule([&]() -> co::Task<void> {
    co_await func();
  });
  co::Scheduler::this_thread().run();
  co::testing::destroy_all_schedulers();
}

// -------------------------------------------------------------------------- //

TEST(ServerResource, RegisterAndConstructFromFactory) {
  run([]() -> co::Future<void> {
    const std::type_info& info = typeid(TestServerResource<std::string>);
    auto factory_func = []() {
      return std::make_unique<TestServerResource<std::string>>("foobar");
    };
    register_server_resource(
      info,
      std::unique_ptr<ServerResourceFactoryBase>{
        new ServerResourceFactory<TestServerResource<std::string>>{
          std::move(factory_func)
        }
      }
    );

    ServerResourceContext context;
    auto resource_ptr = co_await construct_server_resource(info, context);
    TestServerResource<std::string>& resource =
      static_cast<TestServerResource<std::string>&>(*resource_ptr);

    EXPECT_EQ(resource.value, "foobar");
  });
}

TEST(ServerResource, CanRegisterUsingMacro) {
  run([]() -> co::Future<void> {
    const std::type_info& info = typeid(TestServerResource<int>);
    ServerResourceContext context;

    {
      auto resource_ptr = co_await construct_server_resource(info, context);
      TestServerResource<int>& resource =
        static_cast<TestServerResource<int>&>(*resource_ptr);
      EXPECT_EQ(resource.value, 1);
    }

    {
      auto resource_ptr = co_await construct_server_resource(info, context);
      TestServerResource<int>& resource =
        static_cast<TestServerResource<int>&>(*resource_ptr);
      EXPECT_EQ(resource.value, 2)
        << "Constructing a second time should call factory again.";
    }
  });
}

TEST(ServerResource, RecursivelyConstructDependencies) {
  run([]() -> co::Future<void> {
    const std::type_info& info = typeid(DependentTestServerResource<int>);

    int first_value = -1;
    {
      ServerResourceContext context;
      auto resource_ptr = co_await construct_server_resource(info, context);
      DependentTestServerResource<int>& resource =
        static_cast<DependentTestServerResource<int>&>(*resource_ptr);
      EXPECT_GT(resource.value, 0);
      first_value = resource.value;
    }

    {
      ServerResourceContext context;
      auto resource_ptr = co_await construct_server_resource(info, context);
      DependentTestServerResource<int>& resource =
        static_cast<DependentTestServerResource<int>&>(*resource_ptr);
      EXPECT_NE(resource.value, first_value)
        << "Constructing a second time should call factory again.";
    }
  });
}

TEST(ServerResource, ResolvesConvergentDependencyTrees) {
  run([]() -> co::Future<void> {
    const std::type_info& info =
      typeid(DependencyConvergenceTestServerResource);

    ServerResourceContext context;
    auto resource_ptr = co_await construct_server_resource(info, context);
    DependencyConvergenceTestServerResource& resource =
      static_cast<DependencyConvergenceTestServerResource&>(*resource_ptr);
    EXPECT_EQ(
      context.unsafe_get<DependentTestServerResource<int>>().value,
      context.unsafe_get<OtherDependentTestServerResource>().value
    );
    EXPECT_EQ(
      resource.value,
      context.unsafe_get<DependentTestServerResource<int>>().value *
      context.unsafe_get<OtherDependentTestServerResource>().value
    );
  });
}

TEST(ServerResource, AsynchronousResources) {
  run([]() -> co::Future<void> {
    const std::type_info& info = typeid(AsyncTestServerResource<int>);

    int first_value = -1;
    {
      ServerResourceContext context;
      auto resource_ptr = co_await construct_server_resource(info, context);
      AsyncTestServerResource<int>& resource =
        static_cast<AsyncTestServerResource<int>&>(*resource_ptr);
      EXPECT_GT(resource.value, 0);
      first_value = resource.value;
    }

    {
      ServerResourceContext context;
      auto resource_ptr = co_await construct_server_resource(info, context);
      AsyncTestServerResource<int>& resource =
        static_cast<AsyncTestServerResource<int>&>(*resource_ptr);
      EXPECT_NE(resource.value, first_value)
        << "Constructing a second time should call factory again.";
    }
  });
}

}
}
