#include "lw/net/socket.h"

#include <iterator>

#include "gtest/gtest.h"
#include "lw/err/error.h"
#include "lw/memory/buffer.h"

namespace lw::net {
namespace {

TEST(Socket, ConnectToHostAndPort) {
  Socket sock;
  ASSERT_FALSE(sock.is_open());
  sock.connect({.hostname = "www.google.com", .service="80"}).get();
  ASSERT_TRUE(sock.is_open());
  sock.close();
  ASSERT_FALSE(sock.is_open());
}

TEST(Socket, ConnectToHostAndServiceName) {
  Socket sock;
  ASSERT_FALSE(sock.is_open());
  sock.connect({.hostname = "www.google.com", .service="http"}).get();
  ASSERT_TRUE(sock.is_open());
  sock.close();
  ASSERT_FALSE(sock.is_open());
}

TEST(Socket, SendAndReceive) {
  Socket sock;
  sock.connect({.hostname = "www.google.com", .service="80"}).get();

  const char request[] =
    "GET / HTTP/1.1\r\n"
    "Host: www.google.com\r\n"
    "Connection: close\r\n"
    "\r\n";
  Buffer send_buff{std::begin(request), std::end(request)};
  std::size_t sent = sock.send(send_buff).get();
  EXPECT_EQ(sent, send_buff.size());

  Buffer receive_buff{1024};
  std::size_t received = sock.receive(&receive_buff).get();
  ASSERT_GT(received, 15);

  std::string res{reinterpret_cast<const char*>(receive_buff.data()), 15};
  EXPECT_EQ(res, "HTTP/1.1 200 OK");
}

}
}
