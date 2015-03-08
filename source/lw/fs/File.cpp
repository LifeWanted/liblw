
#include <cstring>
#include <uv.h>

#include "lw/fs/File.hpp"

namespace lw {
namespace fs {

File::File( event::Loop& loop ):
    m_loop( loop ),
    m_handle( (uv_fs_s*)malloc( sizeof( uv_fs_s ) ) ),
    m_promise( nullptr ),
    m_file_descriptor( -1 ),
    m_write_buffer{ 0 },
    m_uv_write_buffer( (uv_buf_t*)malloc( sizeof( uv_buf_t ) ) )
{
    m_handle->data = (void*)this;
}

// -------------------------------------------------------------------------- //

File::~File( void ){
    if( m_file_descriptor ){
        uv_fs_close( m_loop.lowest_layer(), m_handle, m_file_descriptor, nullptr );
    }

    free( m_uv_write_buffer );
    free( m_handle );
}

// -------------------------------------------------------------------------- //

event::Future< int > File::open( const std::string& path, const std::ios::openmode mode ){
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
        &File::_open_cb
    );

    return _reset_promise();
}

// -------------------------------------------------------------------------- //

void File::_open_cb( uv_fs_s* handle ){
    File* file = (File*)handle->data;
    file->m_file_descriptor = handle->result;
    file->m_promise->resolve( 1 );
}

// -------------------------------------------------------------------------- //

event::Future< int > File::close( void ){
    uv_fs_close(
        m_loop.lowest_layer(),
        m_handle,
        m_file_descriptor,
        &File::_close_cb
    );
    return _reset_promise();
}

// -------------------------------------------------------------------------- //

void File::_close_cb( uv_fs_s* handle ){
    File* file = (File*)handle->data;
    file->m_file_descriptor = -1;
    file->m_promise->resolve( 1 );
}

// -------------------------------------------------------------------------- //

event::Future< int > File::write( const std::string& str ){
    std::memcpy( m_write_buffer, str.c_str(), str.size() + 1 );
    *m_uv_write_buffer = uv_buf_init( (char*)m_write_buffer, str.size() );

    uv_fs_write(
        m_loop.lowest_layer(),
        m_handle,
        m_file_descriptor,
        m_uv_write_buffer,
        1,
        -1,
        &File::_write_cb
    );

    return _reset_promise();
}

// -------------------------------------------------------------------------- //

void File::_write_cb( uv_fs_s* handle ){
    File* file = (File*)handle->data;
    file->m_promise->resolve( 1 );
}

// -------------------------------------------------------------------------- //

event::Future< int > File::_reset_promise( void ){
    m_promise = std::make_unique< event::Promise< int > >();
    return m_promise->future();
}

// -------------------------------------------------------------------------- //

event::Future< File > open(
    event::Loop& loop,
    const std::string& path,
    const std::ios::openmode mode
){
    auto file = std::make_shared< File >( loop );
    return file->open( path, mode )
        .then< File >([ file ]( int, event::Promise< File >&& promise ){
            promise.resolve( std::move( *file ) );
        })
    ;
}

}
}
