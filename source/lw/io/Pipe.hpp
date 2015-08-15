#pragma once

#include "lw/event.hpp"

struct uv_connect_s;

namespace lw {
namespace io {

LW_DEFINE_EXCEPTION_EX(PipeError, ::lw::event, StreamError);

// ---------------------------------------------------------------------------------------------- //

class Pipe : public event::BasicStream {
public:
    struct ipc_t {};
    static const ipc_t ipc;

    enum Pipes {
        STDIN   = 0,
        STDOUT  = 1,
        STDERR  = 2
    };

    Pipe(event::Loop& loop);

    Pipe(event::Loop& loop, const ipc_t&);

    void open(const int pipe);

    void bind(const std::string& name);

    event::Future<> connect(const std::string& name);

private:
    static std::shared_ptr<_State> _make_state(event::Loop& loop, const bool ipc);

    event::Promise<> m_connect_promise;
    std::shared_ptr<uv_connect_s> m_connect_req;
};

}
}
