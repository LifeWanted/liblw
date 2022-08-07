#pragma once

#include <memory>
#include <optional>
#include <string>

#include "lw/co/task.h"
#include "lw/io/co/co.h"
#include "lw/http/internal/http_mount_path.h"
#include "lw/http/rest/rest_handler.h"
#include "lw/net/router.h"
#include "lw/net/tls_router.h"

namespace lw {
namespace internal {

struct RestRouterOptions {
  std::string default_content_type;
  net::TLSOptions tls;
};

class RestRouterImpl: public net::Router {
public:
  void attach_routes() override;
  co::Task run(std::unique_ptr<io::CoStream> conn) override;
  std::size_t connection_count() const override { return _connection_counter; }

  co::Future<void> run_once(io::CoStream& conn);

private:
  http::internal::EndpointTrie<BaseRestHandlerFactory> _trie;
  std::size_t _connection_counter = 0;
};

}

using RestRouter = ::lw::net::TLSRouter<
  ::lw::internal::RestRouterImpl,
  ::lw::internal::RestRouterOptions
>;

class RestRoute: public net::RouteBase {
public:
  template <typename RestHandlerFactoryType>
  explicit RestRoute(RestHandlerFactoryType&& factory);

  const BaseRestHandlerFactory& factory() const { return *_factory; }

private:
  std::unique_ptr<BaseRestHandlerFactory> _factory;
};

}
