#include "lw/context/context.h"

namespace lw {
namespace {

class ThreadRootContext : public Context {
public:
  static ThreadRootContext& this_thread() {
    static thread_local ThreadRootContext root_ctx;
    return root_ctx;
  }

private:
  ThreadRootContext();
  ~ThreadRootContext() = default;
};

}

namespace internal {

}

}
