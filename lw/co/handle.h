#pragma once

#include <cstddef>

namespace lw::co {

class CoroutineHandle {
public:
  constexpr CoroutineHandle() noexcept {}
  constexpr CoroutineHandle(std::nullptr_t) noexcept {}

  CoroutineHandle& operator=(std::nullptr_t) noexcept {
    _func_ptr = nullptr;
    return *this;
  }



private:
  void* _func_ptr = nullptr;
};

}
