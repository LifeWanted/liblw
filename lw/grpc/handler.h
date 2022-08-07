#pragma once

#include <exception>

#include "grpcpp/grpcpp.h"
#include "lw/base/strings.h"
#include "lw/co/future.h"
#include "lw/err/canonical.h"
#include "lw/grpc/error.h"
#include "lw/grpc/internal/endpoint.h"
#include "lw/grpc/internal/queue_processor.h"

#define LW_REGISTER_GRPC_HANDLER(Service, Endpoint, Handler)      \
  ::lw::grpc::internal::Endpoint<Service::AsyncService, Handler>  \
    grpc_endpoint_ ## Handler{&Service::AsyncService::Request ## Endpoint}

namespace lw::grpc {

class BaseHandler: public internal::QueueProcessor {
public:
  BaseHandler();
  ~BaseHandler() = default;

  ::grpc::ServerContext& grpc_server_context() { return _grpc_server_context; }
  void grpc_queue_tick() override;

protected:
  virtual co::Future<void> do_run() = 0;

private:
  enum class State {
    INITIAL,
    RECEIVING,
    RESPONDED,
    COMPLETE
  };

  State _state = State::INITIAL;
  ::grpc::ServerContext _grpc_server_context;
};

template <typename Request, typename Response>
class Handler: public BaseHandler {
public:
  typedef Request request_type;
  typedef Response response_type;

  Handler(): _grpc_responder{grpc_server_context()} {}
  ~Handler() = default;

  /**
   * Implement this method in subclasses of `Handler` in order to handle
   * requests.
   */
  virtual co::Future<void> run() = 0;

  const Request& request() const { return _request; }
  const Response& response() const { return _response; }
  Response& response() { return _response; }

  Request& grpc_request() { return _request; }
  ::grpc::ServerAsyncResponseWriter<Response>& grpc_responder() {
    return _grpc_responder;
  }

private:
  co::Future<void> do_run() override {
    try {
      co_await run();
      responder.Finish(_response, /*status=*/{}, this);
    } catch (const CanonicalError& err) {
      responder.Finish(/*msg=*/{}, to_grpc_status(err), this);
    } catch (const std::exception& err) {
      responder.Finish(
        /*msg=*/{},
        ::grpc::Status{
          ::grpc::StatusCode::UNKNOWN,
          cat("Unknown server error: ", err.what())
        },
        this
      );
      std::rethrow_exception(std::current_exception());
    } catch (...) {
      responder.Finish(
        /*msg=*/{},
        ::grpc::Status{::grpc::StatusCode::UNKNOWN, "Unknown server error."},
        this
      );
      std::rethrow_exception(std::current_exception());
    }
  }

  Request _request;
  Response _response;
  ::grpc::ServerAsyncResponseWriter<Response> _grpc_responder;
}

}
