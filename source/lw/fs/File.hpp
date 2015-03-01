#pragma once

#include <functional>
#include <ios>
#include <memory>
#include <string>

#include "lw/event/Loop.hpp"
#include "lw/event/Promise.hpp"

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
        open( path, mode ).then( std::forward< Func >( func ) );
    }

    event::Future open( const std::string& path ){
        return open( path, std::ios::in | std::ios::out );
    }

    event::Future open( const std::string& path, const std::ios::openmode mode );

private:
    static void _handle_cb( uv_fs_s* handle );

    void _open( const std::string& path, const std::ios::openmode mode );

    event::Loop& m_loop;
    uv_fs_s* m_handle;
    std::unique_ptr< event::Promise > m_promise;
};

}
}
