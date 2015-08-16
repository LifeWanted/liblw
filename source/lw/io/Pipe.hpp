#pragma once

#include "lw/event.hpp"

struct uv_connect_s;

namespace lw {
namespace io {

LW_DEFINE_EXCEPTION_EX(PipeError, ::lw::event, StreamError);

// ---------------------------------------------------------------------------------------------- //

/// @brief A local process-to-process pipe.
class Pipe : public event::BasicStream {
public:
    struct ipc_t {};
    static const ipc_t ipc;

    /// @brief Well known pipe descriptors.
    enum descriptors {
        IN  = 0,    ///< stdin
        OUT = 1,    ///< stdout
        ERR = 2     ///< stderr
    };

    // ------------------------------------------------------------------------------------------ //

    /// @brief Constructs a standard pipe.
    ///
    /// @param loop The even loop for the pipe.
    Pipe(event::Loop& loop);

    /// @brief Constructs a pipe that can be used to pass handles.
    ///
    /// @param loop The even loop for the pipe.
    Pipe(event::Loop& loop, const ipc_t&);

    // ------------------------------------------------------------------------------------------ //

    /// @brief Opens a pipe on an existing pipe descriptor.
    ///
    /// @param pipe The pipe descriptor to open.
    void open(const int pipe);

    /// @brief Creates a new named pipe/Unix socket and sets this process as the owner.
    ///
    /// @param name The name of the pipe, such as `/tmp/my-awesome-pipe`.
    void bind(const std::string& name);

    /// @brief Connects to an existing named pipe/Unix socket.
    ///
    /// @param name The name of the pipe to connect to.
    ///
    /// @return A promise to be connected.
    event::Future<> connect(const std::string& name);

    // ------------------------------------------------------------------------------------------ //

private:
    /// @brief Constructs the internal shared state for the pipe.
    ///
    /// @param loop The event loop that the pipe will use.
    /// @param ipc  Flag indicating if this pipe will be used to pass handles.
    ///
    /// @return A new shared state for a pipe.
    static uv_stream_s* _make_state(event::Loop& loop, const bool ipc);

    // ------------------------------------------------------------------------------------------ //

    event::Promise<> m_connect_promise;             ///< Promise for making connections.
    std::shared_ptr<uv_connect_s> m_connect_req;    ///< Connection request handle.
};

}
}
