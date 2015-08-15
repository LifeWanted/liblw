
#include <cstdlib>
#include <uv.h>

#include "lw/error.hpp"
#include "lw/io/Pipe.hpp"

namespace lw {
namespace io {

const Pipe::ipc_t Pipe::ipc;

// ---------------------------------------------------------------------------------------------- //

Pipe::Pipe(event::Loop& loop):
    event::BasicStream(_make_state(loop, false))
{}

// ---------------------------------------------------------------------------------------------- //

Pipe::Pipe(event::Loop& loop, const ipc_t&):
    event::BasicStream(_make_state(loop, true))
{}

// ---------------------------------------------------------------------------------------------- //

void Pipe::open(const int pipe){
    int res = uv_pipe_open((uv_pipe_t*)&handle(), pipe);
    if (res < 0) {
        throw LW_UV_ERROR(PipeError, res);
    }
}

// ---------------------------------------------------------------------------------------------- //

void Pipe::bind(const std::string& name){
    int res = uv_pipe_bind((uv_pipe_t*)&handle(), name.c_str());
    if (res < 0) {
        throw LW_UV_ERROR(PipeError, res);
    }
}

// ---------------------------------------------------------------------------------------------- //

event::Future<> Pipe::connect(const std::string& name){
    if (m_connect_promise.is_finished() || m_connect_req != nullptr) {
        throw PipeError(1, "Cannot connect a pipe twice.");
    }

    // Set up the connection request.
    m_connect_req = std::shared_ptr<uv_connect_t>(
        (uv_connect_t*)std::malloc(sizeof(uv_connect_t)),
        &std::free
    );
    m_connect_req->data = (void*)this;

    // Start the connection.
    // NOTE: uv_pipe_connect does not return a status code, its sent to the callback.
    uv_pipe_connect(
        m_connect_req.get(),
        (uv_pipe_t*)&handle(),
        name.c_str(),
        [](uv_connect_t* req, int status){
            Pipe& pipe = *(Pipe*)req->data;
            if (status < 0) {
                pipe.m_connect_promise.reject(LW_UV_ERROR(PipeError, status));
            }
            else {
                pipe.m_connect_promise.resolve();
            }
        }
    );

    // Return a future.
    return m_connect_promise.future();
}

// ---------------------------------------------------------------------------------------------- //

std::shared_ptr<event::BasicStream::_State> Pipe::_make_state(event::Loop& loop, const bool ipc){
    auto state = std::make_shared<_State>();
    state->handle = (uv_stream_t*)std::malloc(sizeof(uv_pipe_t));
    uv_pipe_init(loop.lowest_layer(), (uv_pipe_t*)state->handle, ipc);
    return state;
}

}
}
