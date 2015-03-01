#pragma once

#include <functional>
#include <ios>
#include <string>

#include "lw/event/Loop.hpp"

struct uv_fs_s;

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
        open( path, mode, std::forward< Func >( func ) );
    }

    template< typename Func >
    void open( const std::string& path, Func&& func ){
        open( path, std::ios::in | std::ios::out, std::forward< Func >( func ) );
    }

    template< typename Func >
    void open( const std::string& path, const std::ios::openmode mode, Func&& func ){
        m_callback = std::forward< Func >( func );
        _open( path, mode );
    }

private:
    static void _handle_cb( uv_fs_s* handle );

    void _open( const std::string& path, const std::ios::openmode mode );

    typedef std::function< void( void ) > Callback;
    event::Loop& m_loop;
    uv_fs_s* m_handle;
    File::Callback m_callback;
};

}
}
