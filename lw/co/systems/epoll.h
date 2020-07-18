#pragma once

#include <sys/epoll.h>

namespace lw::co::internal {

class EPoll {
public:
  EPoll();
  EPoll(EPoll&&) = default;
  EPoll& operator=(EPoll&&) = default;
  EPoll(const EPoll&) = delete;
  EPoll& operator=(const EPoll&) = delete;

private:
  int epoll_fd;

};

}
