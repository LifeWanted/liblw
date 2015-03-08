#pragma once

#include <functional>
#include <ios>
#include <memory>
#include <string>

#include "lw/event.hpp"

struct uv_fs_s;
struct uv_buf_t;

namespace lw {
namespace fs {

/// @brief Wraps file access in a clean, promise-friendly package.
class File {
public:
    /// @brief Constructs an unopened file associated with the given event loop.
    ///
    /// @param loop The event loop to use for all file-related events.
    File( event::Loop& loop );

    // ---------------------------------------------------------------------- //

    /// @brief Destructor will close the file if it is open.
    ~File( void );

    // ---------------------------------------------------------------------- //

    /// @brief Asynchronously opens a file handle.
    ///
    /// @param path The path to the file to open.
    /// @param mode The mode to open with (default is `in` and `out`).
    ///
    /// @return A promise to have the file opened.
    event::Future< void > open(
        const std::string& path,
        const std::ios::openmode mode = std::ios::in | std::ios::out
    );

    // ---------------------------------------------------------------------- //

    /// @brief Asynchronously closes the file handle.
    ///
    /// @return A promise to close the file.
    event::Future< void > close( void );

    // ---------------------------------------------------------------------- //

    /// @brief Asynchronously writes data to the file.
    ///
    /// @param str The data to write.
    ///
    /// @return A promise to have the data written.
    event::Future< void > write( const std::string& str );

    // ---------------------------------------------------------------------- //

private:
    /// @brief Handler for open requests.
    static void _open_cb( uv_fs_s* handle );

    // ---------------------------------------------------------------------- //

    /// @brief Handler for close requests.
    static void _close_cb( uv_fs_s* handle );

    // ---------------------------------------------------------------------- //

    /// @brief Handler for write requests.
    static void _write_cb( uv_fs_s* handle );

    // ---------------------------------------------------------------------- //

    /// @brief Creates a new promise and returns the associate future.
    ///
    /// @return The future half of the new promise.
    event::Future< void > _reset_promise( void );

    // ---------------------------------------------------------------------- //

    event::Loop& m_loop;
    uv_fs_s* m_handle;
    std::unique_ptr< event::Promise< void > > m_promise;
    int m_file_descriptor;
    unsigned char m_write_buffer[ 1024 ];
    uv_buf_t* m_uv_write_buffer;
};

// -------------------------------------------------------------------------- //

/// @brief Asynchronously opens a file.
///
/// @param loop The event loop to open the file with.
/// @param path The path to the file to open.
/// @param mode The file mode to use.
///
/// @return A future file.
event::Future< File > open(
    event::Loop& loop,
    const std::string& path,
    const std::ios::openmode mode = std::ios::in | std::ios::out
);

}
}
