#pragma once

#include <memory>
#include <string>
#include <string_view>

#include "lw/co/future.h"
#include "lw/http/http_request.h"
#include "lw/http/http_response.h"

namespace lw {

class HttpHandler {
public:
  HttpHandler() {}
  virtual ~HttpHandler() = default;

  void
  set_request_response(const HttpRequest& request, HttpResponse& response) {
    _request = &request;
    _response = &response;
  }

  const HttpRequest& request() const { return *_request; }
  HttpResponse& response() { return *_response; }

  virtual co::future<void> del() { return _default_behavior(); }
  virtual co::future<void> get() { return _default_behavior(); }
  virtual co::future<void> head() { return _default_behavior(); }
  virtual co::future<void> options() { return _default_behavior(); }
  virtual co::future<void> patch() { return _default_behavior(); }
  virtual co::future<void> post() { return _default_behavior(); }
  virtual co::future<void> put() { return _default_behavior(); }

private:
  co::future<void> _default_behavior();

  const HttpRequest* _request = nullptr;
  HttpResponse* _response = nullptr;
};

class BaseHttpHandlerFactory {
public:
  explicit BaseHttpHandlerFactory(std::string_view route) : _route{route} {}
  virtual ~BaseHttpHandlerFactory() = default;

  std::string_view route() const { return _route; }

  virtual std::unique_ptr<HttpHandler>
  make_handler(const HttpRequest& request, HttpResponse& response) const = 0;

private:
  std::string _route;
};

template <typename HandlerType>
class HttpHandlerFactory : public BaseHttpHandlerFactory {
public:
  explicit HttpHandlerFactory(std::string_view route)
      : BaseHttpHandlerFactory{route} {}

  std::unique_ptr<HttpHandler> make_handler(
      const HttpRequest& request, HttpResponse& response) const override {
    auto handler = std::make_unique<HandlerType>();
    handler->set_request_response(request, response);
    return handler;
  }
};

} // namespace lw
