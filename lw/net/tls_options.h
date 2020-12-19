#pragma once

#include <filesystem>
#include <variant>

namespace lw::net {

struct TLSOptions {
  // TODO(alaina): Add utility for self-signed certs in memory and support for
  // passing a private key here.
  std::variant<std::filesystem::path> private_key;
  std::variant<std::filesystem::path> certificate;

  enum ConnectionMode {
    CLIENT, // Initiator of TLS connection.
    SERVER  // Acceptor of TLS connection.
  };
  ConnectionMode connection_mode = CLIENT;
};

}
