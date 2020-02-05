#include "lw/co/future.h"

#include "gtest/gtest.h"
#include "lw/err/canonical.h"

namespace lw::co {
namespace {

TEST(MakeFuture, CanResolveToAValue) {
  std::future<int> f = make_future(123);
  EXPECT_TRUE(f.valid());
  EXPECT_EQ(f.get(), 123);
}

TEST(MakeFuture, CanRejectAValue) {
  std::exception_ptr e;
  try {
    throw InvalidArgument() << "Some kind of error.";
  } catch (...) {
    e = std::current_exception();
  }

  std::future<int> f = make_future<int>(e);
  EXPECT_TRUE(f.valid());
  EXPECT_THROW(f.get(), InvalidArgument);
}

}
}
