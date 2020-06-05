#pragma once

#include <coroutine>

namespace lw::co {

template <typename T>
class task {
public:
  class promise;
  typedef promise promise_type;

  task();

  bool resume();

private:
};


template <typename T>
class task<T>::promise {

};


}
