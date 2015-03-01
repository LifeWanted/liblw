#pragma once

#include <uv.h>

#include "lw/fs/File.hpp"

namespace lw {
namespace fs {

File::File( event::Loop& loop ):
    m_loop( loop ),
    m_handle( (uv_fs_s*)malloc( sizeof( uv_fs_s ) ) ),
    m_promise( nullptr )
{
    m_handle->data = (void*)this;
}

event::Future File::open( const std::string& path, const std::ios::openmode mode ){
    _open( path, mode );
    m_promise = std::make_unique< event::Promise >();
    return m_promise->future();
}

void File::_open( const std::string& path, const std::ios::openmode mode ){
    int flags = O_CREAT
        | (mode & std::ios::app     ? O_APPEND  : 0)
        | (mode & std::ios::trunc   ? O_TRUNC   : 0)
    ;
    int permissions = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

    if( mode & std::ios::in && mode & std::ios::out ){
        flags |= O_RDWR;
    }
    else if( mode & std::ios::in ){
        flags |= O_RDONLY;
    }
    else if( mode & std::ios::out ){
        flags |= O_WRONLY;
    }

    uv_fs_open(
        m_loop.lowest_layer(),
        m_handle,
        path.c_str(),
        flags,
        permissions,
        &File::_handle_cb
    );
}

void File::_handle_cb( uv_fs_s* handle ){
    ((File*)handle->data)->m_promise->resolve();
}

}
}
