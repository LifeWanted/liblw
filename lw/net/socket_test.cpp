#include "lw/net/socket.h"

#include <chrono>
#include <iterator>


#include <iostream>


#include "gtest/gtest.h"
#include "lw/co/scheduler.h"
#include "lw/co/testing/destroy_scheduler.h"
#include "lw/co/time.h"
#include "lw/err/error.h"
#include "lw/memory/buffer.h"

namespace lw::net {
namespace {

using ::lw::co::testing::destroy_all_schedulers;

class SocketTest : public ::testing::Test {
protected:
  ~SocketTest() {
    destroy_all_schedulers();
  }

  co::Scheduler& scheduler() const {
    return co::Scheduler::this_thread();
  }

  std::string to_string(const Buffer& buff) const {
    return std::string{buff.begin(), buff.end()};
  }
};

TEST_F(SocketTest, ConnectToHostAndPort) {
  scheduler().schedule([]() -> co::Task<void> {
    Socket sock;
    EXPECT_FALSE(sock.is_open());
    co_await sock.connect({.hostname = "www.google.com", .service = "80"});
    EXPECT_TRUE(sock.is_open());
    sock.close();
    EXPECT_FALSE(sock.is_open());
  });
  scheduler().run();
}

TEST_F(SocketTest, ConnectToHostAndServiceName) {
  scheduler().schedule([]() -> co::Task<void> {
    Socket sock;
    EXPECT_FALSE(sock.is_open());
    co_await sock.connect({.hostname = "www.google.com", .service = "http"});
    EXPECT_TRUE(sock.is_open());
    sock.close();
    EXPECT_FALSE(sock.is_open());
  });
  scheduler().run();
}

TEST_F(SocketTest, SendAndReceive) {
  scheduler().schedule([]() -> co::Task<void> {
    Socket sock;
    co_await sock.connect({.hostname = "www.google.com", .service = "80"});

    const char request[] =
      "GET / HTTP/1.1\r\n"
      "Host: www.google.com\r\n"
      "Connection: close\r\n"
      "\r\n";
    Buffer send_buff{std::begin(request), std::end(request)};
    std::size_t sent = co_await sock.send(send_buff);
    EXPECT_EQ(sent, send_buff.size());

    Buffer receive_buff{1024};
    std::size_t received = co_await sock.receive(receive_buff);
    EXPECT_GT(received, 15);

    std::string res{reinterpret_cast<const char*>(receive_buff.data()), 15};
    EXPECT_EQ(res, "HTTP/1.1 200 OK");
  });
  scheduler().run();
}

TEST_F(SocketTest, AcceptConnection) {
  Buffer send_buff{14};
  Buffer receive_buff{send_buff.size()};

  // TODO(alaina): This pointer is needed due to what I believe to be an error
  // in GCC around capturing coroutine variables. For some reason if I capture
  // by reference in the lambda, then the changes to the buffer do not propagate
  // outward. It seems as though a copy of the buffer is made for the coroutine
  // instead of the reference propagating.
  Buffer* receive_buff_ptr = &receive_buff;
  send_buff.copy("Hello, World!", 14);
  ASSERT_NE(send_buff, receive_buff);
  Address addr{.hostname = "localhost", .service = "8080"};

  scheduler().schedule([=, this]() -> co::Task<void> {
    Socket server;
    server.listen(addr);

    Socket client = co_await server.accept();
    std::size_t received_bytes = co_await client.receive(*receive_buff_ptr);
    EXPECT_EQ(received_bytes, receive_buff_ptr->size());
  });
  scheduler().schedule([&]() -> co::Task<void> {
    Socket client;
    co_await client.connect(addr);
    EXPECT_EQ(co_await client.send(send_buff), send_buff.size());
  });
  scheduler().run();

  EXPECT_EQ(to_string(send_buff), to_string(receive_buff));
}

}
}
