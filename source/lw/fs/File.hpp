#pragma once

#include <functional>
#include <ios>
#include <memory>
#include <string>

#include "lw/event/Loop.hpp"
#include "lw/event/Promise.hpp"

struct uv_fs_s;
struct uv_buf_t;

namespace lw {
namespace fs {

class File {
public:
    File( event::Loop& loop );

    template< typename Func >
    File(
        event::Loop& loop,
        const std::string& path,
        const std::ios::openmode mode,
        Func&& func
    ):
        File( loop )
    {
        open( path, mode ).then( std::forward< Func >( func ) );
    }

    ~File( void );

    event::Future open( const std::string& path, const std::ios::openmode mode = std::ios::in | std::ios::out );

    event::Future close( void );

    event::Future write( const std::string& str );

private:
    static void _open_cb( uv_fs_s* handle );
    static void _close_cb( uv_fs_s* handle );
    static void _write_cb( uv_fs_s* handle );

    event::Future _reset_promise( void );
    void _open( const std::string& path, const std::ios::openmode mode );

    event::Loop& m_loop;
    uv_fs_s* m_handle;
    std::unique_ptr< event::Promise > m_promise;
    int m_file_descriptor;
    unsigned char m_write_buffer[ 1024 ];
    uv_buf_t* m_uv_write_buffer;
};

}
}
