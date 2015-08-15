#pragma once

#include <functional>
#include <list>
#include <type_traits>

#include "lw/error.hpp"
#include "lw/event/Promise.hpp"
#include "lw/memory.hpp"

struct uv_stream_s;

namespace lw {
namespace event {

LW_DEFINE_EXCEPTION(StreamError);

/// @brief Base class for asynchronous streams.
class BasicStream {
public:
    /// @brief With streams, all buffers must be pointers.
    typedef std::shared_ptr<const memory::Buffer> buffer_ptr_t;

    /// @brief Read callback functor type.
    ///
    /// @param stream   The stream performing the read.
    /// @param buffer   The buffer containing the read data.
    typedef std::function<void(BasicStream& stream, buffer_ptr_t buffer)> read_callback_t;

    // ------------------------------------------------------------------------------------------ //

    virtual ~BasicStream(void){}

    // ------------------------------------------------------------------------------------------ //

    /// @brief Starts the stream reading.
    ///
    /// Once read starts, the stream will continue reading until it reaches the end, encounters an
    /// error, or is told to stop via `stop_read`.
    ///
    /// The returned promise will be resolved if the stream reaches the end or if `stop_read` is
    /// called. It will be rejected if any error occurs.
    ///
    /// @tparam Func A functor matching the `read_callback_t`.
    ///
    /// @param func The functor to call when there is data available.
    ///
    /// @return A promise for the total number of bytes read.
    template< typename Func >
    Future< std::size_t > read( Func&& func ){
        static_assert(
            std::is_convertible< Func, read_callback_t >::value,
            "`Func` must be compatible with `BasicStream::read_callback_t`."
        );
        auto state = m_state;
        m_state->read_callback = [ state, func ](
            BasicStream& stream,
            buffer_ptr_t buffer
        ){
            func( stream, buffer );
        };
        return _read();
    }

    // ------------------------------------------------------------------------------------------ //

    /// @brief Stops the active read, and resolves the associated promise.
    void stop_read( void );

    // ------------------------------------------------------------------------------------------ //

    /// @brief Writes the data in the given buffer.
    ///
    /// @param buffer The data to write.
    ///
    /// @return A promise to write the data. The value will be the number of
    ///         bytes written.
    Future< std::size_t > write( buffer_ptr_t buffer );

    // ------------------------------------------------------------------------------------------ //

protected:
    /// @brief The internal stream state.
    struct _State : public std::enable_shared_from_this< _State >{
        ~_State( void );

        uv_stream_s*    handle;         ///< The underlying stream handle.
        std::size_t     read_count;     ///< The running tally of bytes read.
        read_callback_t read_callback;  ///< The functor to call with read data.

        Promise<std::size_t> read_promise;              ///< The read promise.
        std::list<memory::Buffer> idle_read_buffers;    ///< List of available read buffers.
        std::list<memory::Buffer> active_read_buffers;  ///< List of in-use read buffers.
    };

    // ------------------------------------------------------------------------------------------ //

    /// @brief Constructs the stream around the low-level libuv handle.
    ///
    /// The provided handle will be freed once the stream is done.
    ///
    /// @param handle The stream handle.
    BasicStream( uv_stream_s* handle );

    // ------------------------------------------------------------------------------------------ //

    /// @brief Reconstructs a `BasicStream` from its internal state.
    ///
    /// @param state The stream state to wrap up.
    BasicStream( const std::shared_ptr< _State >& state ):
        m_state( state )
    {}

    // ------------------------------------------------------------------------------------------ //

    /// @brief Retrieves the stream handle.
    uv_stream_s& handle( void ){
        return *m_state->handle;
    }

    // ------------------------------------------------------------------------------------------ //

    /// @copydoc BasicStream::handle()
    const uv_stream_s& handle( void ) const {
        return *m_state->handle;
    }

    // ------------------------------------------------------------------------------------------ //

    /// @brief Sets the state context for the stream.
    ///
    /// @param state The new stream state.
    void state(const std::shared_ptr<_State>& state);

    // ------------------------------------------------------------------------------------------ //

private:
    std::shared_ptr< _State > m_state; ///< Internal state pointer.

    // ------------------------------------------------------------------------------------------ //

    /// @brief The internal implementation of beginning the read.
    Future< std::size_t > _read( void );

    // ------------------------------------------------------------------------------------------ //

    /// @brief Resolves the read promise and resets the promise and read count.
    void _stop_read( void );

    // ------------------------------------------------------------------------------------------ //

    /// @brief Gets an available read buffer.
    ///
    /// If no buffers are available, then a new one is allocated.
    ///
    /// @return A reference to a useable memory buffer.
    memory::Buffer& _next_read_buffer( void );

    // ------------------------------------------------------------------------------------------ //

    /// @brief Releases the given buffer, making it available to read into again.
    ///
    /// @param base A pointer to the first byte in the buffer.
    void _release_read_buffer( const void* base );
};

}
}
